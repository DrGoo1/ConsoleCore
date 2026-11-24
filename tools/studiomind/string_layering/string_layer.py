#!/usr/bin/env python3
"""Render StudioMind's country-pop string layering rules into MIDI in any key.

Every part is written key-independently in rules.json: chord tones (R, 3, 5)
relative to the current chord, scale degrees relative to the key, or guitar
fret shapes from a shape key (G, C or D) plus a capo chosen from the song key.

Usage:
    python3 string_layer.py --key Bb --out example_Bb.mid --csv example_Bb.csv
    python3 string_layer.py --key E --form "verse:I V vi IV|chorus:IV I V vi"

Standard library only.
"""
from __future__ import annotations

import argparse
import csv
import json
import random
import struct
from pathlib import Path

HERE = Path(__file__).resolve().parent
PPQ = 480
STEP = PPQ // 4  # one 16th note
BAR = STEP * 16

NOTE_NAMES = ["C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"]
KEY_PC = {"C": 0, "C#": 1, "Db": 1, "D": 2, "D#": 3, "Eb": 3, "E": 4, "F": 5,
          "F#": 6, "Gb": 6, "G": 7, "G#": 8, "Ab": 8, "A": 9, "A#": 10, "Bb": 10, "B": 11}
MAJOR = [0, 2, 4, 5, 7, 9, 11]
ROMAN = {"I": (0, "maj"), "ii": (1, "min"), "iii": (2, "min"), "IV": (3, "maj"),
         "V": (4, "maj"), "vi": (5, "min"), "vii": (6, "dim")}
TONE = {"maj": {"R": 0, "3": 4, "5": 7}, "min": {"R": 0, "3": 3, "5": 7},
        "dim": {"R": 0, "3": 3, "5": 6}}

DEFAULT_FORM = ("verse:I V vi IV|chorus:IV I V vi|verse:I V vi IV|"
                "bridge:vi IV|chorus:IV I V vi")


def name(midi: int) -> str:
    return f"{NOTE_NAMES[midi % 12]}{midi // 12 - 1}"


def load_rules(path: Path = HERE / "rules.json") -> dict:
    return json.loads(path.read_text())


def parse_form(form: str) -> list[tuple[str, int, str]]:
    """'verse:I V|chorus:IV I' -> [(section, instance, roman), ...] one per bar."""
    bars, counts = [], {}
    for chunk in form.split("|"):
        section, chords = chunk.split(":")
        section = section.strip()
        counts[section] = counts.get(section, 0) + 1
        for roman in chords.split():
            if roman not in ROMAN:
                raise ValueError(f"unknown chord {roman!r}; use I ii iii IV V vi vii")
            bars.append((section, counts[section], roman))
    return bars


def chord_root_pc(key_pc: int, roman: str) -> int:
    return (key_pc + MAJOR[ROMAN[roman][0]]) % 12


def degree_pc(key_pc: int, degree: int) -> int:
    return (key_pc + MAJOR[(degree - 1) % 7]) % 12


def place(pc: int, low: int) -> int:
    """Lowest MIDI note with pitch class pc at or above low."""
    return low + (pc - low) % 12


def chord_voicing(key_pc: int, roman: str, low: int, count: int = 6) -> list[int]:
    root = chord_root_pc(key_pc, roman)
    pcs = {(root + iv) % 12 for iv in TONE[ROMAN[roman][1]].values()}
    out, p = [], low
    while len(out) < count:
        if p % 12 in pcs:
            out.append(p)
        p += 1
    return out


def tone_pitch(token: str, key_pc: int, roman: str, low: int) -> int:
    if token.startswith("v"):
        return chord_voicing(key_pc, roman, low)[int(token[1:]) - 1]
    up = 12 if token.endswith("+12") else 0
    base = token.replace("+12", "")
    quality = ROMAN[roman][1]
    pc = (chord_root_pc(key_pc, roman) + TONE[quality][base]) % 12
    return place(pc, low) + up


def choose_capos(rules: dict, key_pc: int) -> dict:
    shapes = rules["shape_keys"]
    main_rule = rules["capo_rules"]["main"]
    options = [(s, (key_pc - shapes[s]["offset"]) % 12) for s in main_rule["prefer"]]
    main = min((o for o in options if o[1] <= main_rule["max_capo"]),
               key=lambda o: (o[1], main_rule["prefer"].index(o[0])))
    high_rule = rules["capo_rules"]["high"]
    high_opts = [o for o in options if o[0] != main[0]
                 and high_rule["min_capo"] <= o[1] <= high_rule["max_capo"]]
    high = min(high_opts, key=lambda o: (abs(o[1] - high_rule["target"]), o[1]))
    return {"main": main, "high": high, "nashville": main}


def shape_pitches(rules: dict, shape_key: str, capo: int, roman: str, tuning: str) -> list[int]:
    frets = rules["shape_keys"][shape_key][roman]
    strings = rules["tunings"][tuning]
    return [s + f + capo for s, f in zip(strings, frets) if f >= 0]


class Track:
    def __init__(self, name_: str, program: int, channel: int):
        self.name, self.program, self.channel = name_, program, channel
        self.events: list[tuple[int, int, bytes]] = []  # (tick, order, data)
        self.notes: list[dict] = []

    def note(self, tick: int, dur: int, pitch: int, vel: int, meta: dict):
        tick, dur = max(0, tick), max(10, dur)
        vel = max(1, min(127, vel))
        self.events.append((tick, 1, bytes([0x90 | self.channel, pitch, vel])))
        self.events.append((tick + dur, 0, bytes([0x80 | self.channel, pitch, 0])))
        self.notes.append({"track": self.name, "tick": tick, "dur": dur, "midi": pitch,
                           "note": name(pitch), "vel": vel, **meta})

    def pan(self, tick: int, pan_pct: int):
        value = round(64 + pan_pct / 100 * 63) if pan_pct >= 0 else round(64 + pan_pct / 100 * 64)
        self.events.append((tick, 0, bytes([0xB0 | self.channel, 10, max(0, min(127, value))])))

    def chunk(self) -> bytes:
        body = bytearray()
        evs = [(0, -2, b"\xff\x03" + _vlq(len(self.name.encode())) + self.name.encode()),
               (0, -1, bytes([0xC0 | self.channel, self.program]))] + self.events
        last = 0
        for tick, _, data in sorted(evs, key=lambda e: (e[0], e[1])):
            body += _vlq(tick - last) + data
            last = tick
        body += b"\x00\xff\x2f\x00"
        return b"MTrk" + struct.pack(">I", len(body)) + bytes(body)


def _vlq(n: int) -> bytes:
    out = [n & 0x7F]
    n >>= 7
    while n:
        out.append((n & 0x7F) | 0x80)
        n >>= 7
    return bytes(reversed(out))


def write_midi(path: Path, tracks: list[Track], bpm: float, title: str):
    tempo = round(60_000_000 / bpm)
    meta = bytearray()
    meta += b"\x00\xff\x03" + _vlq(len(title)) + title.encode()
    meta += b"\x00\xff\x51\x03" + tempo.to_bytes(3, "big")
    meta += b"\x00\xff\x58\x04\x04\x02\x18\x08"
    meta += b"\x00\xff\x2f\x00"
    data = b"MThd" + struct.pack(">IHHH", 6, 1, len(tracks) + 1, PPQ)
    data += b"MTrk" + struct.pack(">I", len(meta)) + bytes(meta)
    for t in tracks:
        data += t.chunk()
    path.write_bytes(data)


def render(rules: dict, key: str, form: str = DEFAULT_FORM, bpm: float = 110, seed: int = 7):
    key_pc = KEY_PC[key]
    bars = parse_form(form)
    capos = choose_capos(rules, key_pc)
    parts = {p["id"]: p for p in rules["parts"]}
    ms_to_ticks = PPQ * bpm / 60_000
    stagger = round(rules["strum"]["stagger_ms"] * ms_to_ticks)
    tracks, channel = [], 0

    for part in rules["parts"]:
        base = parts[part["copy_of"]] if "copy_of" in part else part
        if channel == 9:
            channel += 1
        track = Track(part["name"], part["program"], channel)
        channel += 1
        rng = random.Random(f"{seed}-{key}-{part['id']}")
        jitter = round(part.get("humanize_ms", 0) * ms_to_ticks)
        vel_j = part.get("vel_jitter", 0)
        last_section = None
        for bar_i, (section, instance, roman) in enumerate(bars):
            spec = part["sections"].get(section)
            if not spec or ("only_instances" in spec and instance not in spec["only_instances"]):
                last_section = None
                continue
            bar_tick = bar_i * BAR
            if (section, instance) != last_section:
                track.pan(bar_tick, spec["pan"])
                last_section = (section, instance)
            next_roman = bars[bar_i + 1][2] if bar_i + 1 < len(bars) else roman
            pattern = spec["pattern"]
            hits = [i for i, c in enumerate(pattern) if c != "."]
            seq_i = 0
            for h_n, step in enumerate(hits):
                char = pattern[step]
                nxt = hits[h_n + 1] if h_n + 1 < len(hits) else 16
                tick = bar_tick + step * STEP + (rng.randint(-jitter, jitter) if jitter else 0)
                vel = spec["vel"] + (rng.randint(-vel_j, vel_j) if vel_j else 0)
                meta = {"bar": bar_i + 1, "step": step + 1, "section": f"{section}{instance}",
                        "chord": roman}
                mode = base["mode"]
                if mode in ("tones", "degree_tones"):
                    gate = spec.get("x_gate", 0.6) if char == "x" else spec.get("gate", 1.0)
                    if char == "x":
                        vel -= 25
                    dur = round(gate * STEP)
                    if mode == "tones":
                        tokens = base["tones"][seq_i % len(base["tones"])]
                        seq_i += 1
                        tokens = tokens if isinstance(tokens, list) else [tokens]
                        low = base["register"][0]
                        pitches = []
                        for tok in tokens:
                            p = tone_pitch(tok, key_pc, roman, low)
                            if pitches and p <= pitches[-1] and tok[0] != "v" and not tok.endswith("+12"):
                                p += 12 * ((pitches[-1] - p) // 12 + 1)
                            pitches.append(p)
                        if spec.get("approach") and h_n == len(hits) - 1 and next_roman != roman:
                            target = tone_pitch("R", key_pc, next_roman, low)
                            nr_deg = ROMAN[next_roman][0]
                            below = (key_pc + MAJOR[(nr_deg - 1) % 7]) % 12
                            pitches = [target - ((target - below) % 12 or 12)]
                            meta["approach_to"] = next_roman
                    else:
                        deg, octave = base["degrees"][0]
                        pitches = [place(degree_pc(key_pc, deg), base["register"][0]) + 12 * octave]
                    for p in pitches:
                        track.note(tick, dur, p, vel, meta)
                    continue
                # strummed modes
                if mode == "shape_strum":
                    shape_key, capo = capos[base["shape"]]
                    voicing = shape_pitches(rules, shape_key, capo, roman, base["tuning"])
                    meta.update({"shape": f"{shape_key}-shape capo {capo}"})
                elif mode == "degree_strum":
                    root = place(key_pc, base["register"][0])
                    voicing = [root + (MAJOR[(d - 1) % 7]) + 12 * o for d, o in base["degrees"]]
                elif mode == "static_strum":
                    low = base["register"][0]
                    voicing = []
                    for tok in base["tones"]:
                        p = tone_pitch(tok, key_pc, base["chord"], low)
                        if voicing and p <= voicing[-1] and tok[0] != "v" and not tok.endswith("+12"):
                            p += 12
                        voicing.append(p)
                    meta["chord"] = base["chord"] + " (held)"
                else:
                    raise ValueError(mode)
                voicing = sorted(set(voicing))  # unison strings (Nashville) sound once
                if base.get("strings") == "top4":
                    voicing = voicing[-4:]
                if char == "U":
                    strings, vel = list(reversed(voicing[-4:])), vel - 14
                elif char == "d":
                    strings, vel = list(reversed(voicing[-3:])), vel // 2
                else:
                    strings = voicing
                end = bar_tick + (16 if spec.get("hold") else nxt) * STEP
                for s_i, p in enumerate(strings):
                    t = tick + s_i * stagger
                    track.note(t, end - t - 8, p, vel - s_i * 2, meta)
        tracks.append(track)
    return tracks, capos, bars


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--key", default="G", choices=sorted(KEY_PC))
    ap.add_argument("--form", default=DEFAULT_FORM)
    ap.add_argument("--bpm", type=float, default=110)
    ap.add_argument("--out", type=Path)
    ap.add_argument("--csv", type=Path)
    ap.add_argument("--rules", type=Path, default=HERE / "rules.json")
    args = ap.parse_args(argv)
    rules = load_rules(args.rules)
    tracks, capos, bars = render(rules, args.key, args.form, args.bpm)
    out = args.out or Path(f"string_layers_{args.key}.mid")
    write_midi(out, tracks, args.bpm, f"StudioMind string layers in {args.key}")
    print(f"key {args.key}: main pair {capos['main'][0]}-shape capo {capos['main'][1]}, "
          f"high layer {capos['high'][0]}-shape capo {capos['high'][1]}, "
          f"{len(bars)} bars, {sum(len(t.notes) for t in tracks)} notes -> {out}")
    if args.csv:
        rows = sorted((n for t in tracks for n in t.notes), key=lambda n: (n["tick"], n["track"]))
        fields = ["bar", "step", "section", "chord", "track", "note", "midi", "vel", "tick", "dur",
                  "shape", "approach_to"]
        with args.csv.open("w", newline="") as fh:
            w = csv.DictWriter(fh, fieldnames=fields, extrasaction="ignore")
            w.writeheader()
            w.writerows(rows)


if __name__ == "__main__":
    main()
