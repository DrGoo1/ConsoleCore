#!/usr/bin/env python3
"""Song Cage reader for StudioMind.

Given a Song Cage song file (.songcage.json) and the MIDI it exported, print
every lyric syllable against its melody note, its time and the chord under
it, and flag what can't be lined up.

Song Cage does not publish the layout of its song file, so this reader does
not assume one. It walks the JSON looking for lyric-like and chord-like
records using the key names in KEYMAP below. When a real file arrives, run
with --schema to see its layout and adjust KEYMAP (or pass --keymap
my_keys.json) instead of rewriting the code.

Standard library only, Python 3.8+.

Usage:
  python3 songcage_reader.py SONG.songcage.json SONG.mid
  python3 songcage_reader.py SONG.songcage.json SONG.mid --jsonl out.jsonl
  python3 songcage_reader.py SONG.songcage.json --schema
  python3 songcage_reader.py --midi-only SONG.mid
"""
from __future__ import annotations

import argparse
import json
import struct
import sys
from dataclasses import dataclass, field
from typing import Any, Dict, List, Optional, Tuple

# ---------------------------------------------------------------------------
# KEYMAP: the only part that should need editing once a real file is seen.
# All matching is case-insensitive. "ancestor" lists match any part of a key
# path above the record; the other lists match a record's own keys.
# ---------------------------------------------------------------------------
KEYMAP: Dict[str, List[str]] = {
    # a record counts as lyric if some key above it contains one of these
    "lyric_ancestor": ["lyric", "syllab", "word", "compartment", "block"],
    # ...and it has a string field with one of these names
    "text": ["text", "syllable", "lyric", "word", "value", "content", "label"],
    # a record counts as chord if some key above it contains one of these
    "chord_ancestor": ["chord", "harmony", "progression"],
    "chord_name": ["symbol", "name", "label", "chord", "display", "text"],
    "chord_root": ["root"],
    "chord_quality": ["quality", "type", "suffix", "kind"],
    # section records
    "section_ancestor": ["section", "part"],
    "section_name": ["name", "title", "label", "type", "kind"],
    # numeric fields
    "position": ["beat", "start", "startbeat", "position", "pos", "time",
                 "offset", "onset", "tick", "at"],
    "duration": ["duration", "length", "len", "beats", "dur"],
    "pitch": ["pitch", "midi", "note", "notenumber", "midinote", "key"],
    # lists of notes hanging off a syllable (melisma)
    "notes": ["notes", "pitches", "slots", "melody", "melodynotes"],
}

NOTE_NAMES = ["C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"]


def note_name(p: Optional[int]) -> str:
    if p is None:
        return "-"
    return f"{NOTE_NAMES[p % 12]}{p // 12 - 1}"


# ===========================================================================
# MIDI
# ===========================================================================
@dataclass
class MidiNote:
    start: int
    end: int
    pitch: int
    velocity: int
    channel: int


@dataclass
class MidiTrack:
    index: int
    name: str = ""
    programs: List[int] = field(default_factory=list)
    notes: List[MidiNote] = field(default_factory=list)
    lyrics: List[Tuple[int, str]] = field(default_factory=list)
    texts: List[Tuple[int, str]] = field(default_factory=list)


@dataclass
class MidiFile:
    ppq: int
    tracks: List[MidiTrack]
    tempos: List[Tuple[int, int]]          # (tick, microseconds per quarter)
    timesigs: List[Tuple[int, int, int]]   # (tick, numerator, denominator)
    keysigs: List[Tuple[int, int, int]]    # (tick, sharps/flats, minor)

    def seconds(self, tick: int) -> float:
        tempos = self.tempos or [(0, 500000)]
        if tempos[0][0] != 0:
            tempos = [(0, 500000)] + tempos
        t = 0.0
        for i, (tk, us) in enumerate(tempos):
            nxt = tempos[i + 1][0] if i + 1 < len(tempos) else None
            if nxt is not None and tick > nxt:
                t += (nxt - tk) * us / 1e6 / self.ppq
            else:
                t += (tick - tk) * us / 1e6 / self.ppq
                break
        return t

    def bar_beat(self, tick: int) -> Tuple[int, float]:
        """1-based bar and 1-based beat (beat = time-signature denominator)."""
        sigs = self.timesigs or [(0, 4, 4)]
        if sigs[0][0] != 0:
            sigs = [(0, 4, 4)] + sigs
        bar = 1
        for i, (tk, num, den) in enumerate(sigs):
            beat_ticks = self.ppq * 4 / den
            bar_ticks = beat_ticks * num
            nxt = sigs[i + 1][0] if i + 1 < len(sigs) else None
            if nxt is not None and tick >= nxt:
                bars = (nxt - tk) / bar_ticks
                bar += int(round(bars)) if abs(bars - round(bars)) < 1e-6 else int(bars) + 1
                continue
            rel = tick - tk
            bar += int(rel // bar_ticks)
            beat = (rel % bar_ticks) / beat_ticks + 1
            return bar, beat
        return bar, 1.0

    def tick_of(self, bar: float, beat: float) -> float:
        """Tick for a 1-based bar and 1-based beat (inverse of bar_beat)."""
        sigs = self.timesigs or [(0, 4, 4)]
        if sigs[0][0] != 0:
            sigs = [(0, 4, 4)] + sigs
        cur_bar, base = 1.0, 0.0
        for i, (tk, num, den) in enumerate(sigs):
            beat_ticks = self.ppq * 4 / den
            bar_ticks = beat_ticks * num
            nxt = sigs[i + 1][0] if i + 1 < len(sigs) else None
            if nxt is not None:
                bars_here = (nxt - tk) / bar_ticks
                if bar >= cur_bar + bars_here - 1e-9:
                    cur_bar += bars_here
                    base = nxt
                    continue
            return base + (bar - cur_bar) * bar_ticks + (beat - 1) * beat_ticks
        return base

    def tempo_bpm(self) -> Optional[float]:
        return 60e6 / self.tempos[0][1] if self.tempos else None


def _read_varlen(data: bytes, i: int) -> Tuple[int, int]:
    v = 0
    while True:
        b = data[i]
        i += 1
        v = (v << 7) | (b & 0x7F)
        if not b & 0x80:
            return v, i


def read_midi(path: str) -> MidiFile:
    with open(path, "rb") as fh:
        data = fh.read()
    if data[:4] != b"MThd":
        raise ValueError(f"{path} is not a standard MIDI file")
    hlen = struct.unpack(">I", data[4:8])[0]
    _fmt, ntrk, division = struct.unpack(">HHH", data[8:14])
    if division & 0x8000:
        raise ValueError("SMPTE-timed MIDI is not supported")
    ppq = division
    i = 8 + hlen
    tracks: List[MidiTrack] = []
    tempos: List[Tuple[int, int]] = []
    timesigs: List[Tuple[int, int, int]] = []
    keysigs: List[Tuple[int, int, int]] = []
    for ti in range(ntrk):
        if data[i:i + 4] != b"MTrk":
            break
        tlen = struct.unpack(">I", data[i + 4:i + 8])[0]
        j, end = i + 8, i + 8 + tlen
        i = end
        trk = MidiTrack(index=ti)
        tick, status = 0, 0
        open_notes: Dict[Tuple[int, int], List[Tuple[int, int]]] = {}
        while j < end:
            delta, j = _read_varlen(data, j)
            tick += delta
            b = data[j]
            if b & 0x80:
                status = b
                j += 1
            if status == 0xFF:
                mtype = data[j]
                mlen, j = _read_varlen(data, j + 1)
                payload = data[j:j + mlen]
                j += mlen
                text = payload.decode("utf-8", "replace") if mtype in (1, 3, 5) else ""
                if mtype == 0x03 and not trk.name:
                    trk.name = text
                elif mtype == 0x05:
                    trk.lyrics.append((tick, text))
                elif mtype == 0x01:
                    trk.texts.append((tick, text))
                elif mtype == 0x51 and mlen == 3:
                    tempos.append((tick, int.from_bytes(payload, "big")))
                elif mtype == 0x58 and mlen >= 2:
                    timesigs.append((tick, payload[0], 2 ** payload[1]))
                elif mtype == 0x59 and mlen == 2:
                    keysigs.append((tick, struct.unpack("b", payload[:1])[0], payload[1]))
                elif mtype == 0x2F:
                    break
                continue
            if status in (0xF0, 0xF7):
                slen, j = _read_varlen(data, j)
                j += slen
                continue
            kind, ch = status & 0xF0, status & 0x0F
            if kind in (0xC0, 0xD0):
                d1 = data[j]
                j += 1
                if kind == 0xC0:
                    trk.programs.append(d1)
                continue
            d1, d2 = data[j], data[j + 1]
            j += 2
            if kind == 0x90 and d2 > 0:
                open_notes.setdefault((ch, d1), []).append((tick, d2))
            elif kind == 0x80 or (kind == 0x90 and d2 == 0):
                stack = open_notes.get((ch, d1))
                if stack:
                    st, vel = stack.pop(0)
                    trk.notes.append(MidiNote(st, tick, d1, vel, ch))
        for (ch, p), stack in open_notes.items():
            for st, vel in stack:
                trk.notes.append(MidiNote(st, tick, p, vel, ch))
        trk.notes.sort(key=lambda n: (n.start, n.pitch))
        tracks.append(trk)
    tempos.sort()
    timesigs.sort()
    return MidiFile(ppq, tracks, tempos, timesigs, keysigs)


def _polyphony(trk: MidiTrack) -> float:
    if not trk.notes:
        return 0.0
    onsets: Dict[int, int] = {}
    for n in trk.notes:
        onsets[n.start] = onsets.get(n.start, 0) + 1
    return len(trk.notes) / len(onsets)


def pick_tracks(mf: MidiFile, melody_idx: Optional[int] = None,
                chord_idx: Optional[int] = None) -> Tuple[Optional[MidiTrack], Optional[MidiTrack]]:
    with_notes = [t for t in mf.tracks if t.notes]
    by_idx = {t.index: t for t in mf.tracks}

    def by_name(words):
        for t in with_notes:
            if any(w in t.name.lower() for w in words):
                return t
        return None

    melody = by_idx.get(melody_idx) if melody_idx is not None else None
    melody = melody or by_name(["melody", "lead", "vocal", "voice", "tune"])
    if melody is None:  # Song Cage plays melody on Flute (GM program 74 -> 73)
        melody = next((t for t in with_notes if 73 in t.programs), None)
    if melody is None and with_notes:
        melody = min(with_notes, key=_polyphony)
    chord = by_idx.get(chord_idx) if chord_idx is not None else None
    chord = chord or by_name(["chord", "harmony", "guitar", "piano", "rhythm", "accomp"])
    if chord is None or chord is melody:
        others = [t for t in with_notes if t is not melody]
        chord = max(others, key=_polyphony) if others else None
    return melody, chord


# ---------------------------------------------------------------------------
# Chord naming from the chord track
# ---------------------------------------------------------------------------
CHORD_TEMPLATES = [
    ("", {0, 4, 7}), ("m", {0, 3, 7}), ("dim", {0, 3, 6}), ("aug", {0, 4, 8}),
    ("sus2", {0, 2, 7}), ("sus4", {0, 5, 7}), ("7", {0, 4, 7, 10}),
    ("maj7", {0, 4, 7, 11}), ("m7", {0, 3, 7, 10}), ("m7b5", {0, 3, 6, 10}),
    ("dim7", {0, 3, 6, 9}), ("6", {0, 4, 7, 9}), ("m6", {0, 3, 7, 9}),
    ("add9", {0, 2, 4, 7}), ("madd9", {0, 2, 3, 7}), ("9", {0, 2, 4, 7, 10}),
    ("maj9", {0, 2, 4, 7, 11}), ("m9", {0, 2, 3, 7, 10}), ("7sus4", {0, 5, 7, 10}),
    ("5", {0, 7}),
]


def name_chord(pitches: List[int]) -> str:
    if not pitches:
        return "-"
    pcs = {p % 12 for p in pitches}
    bass = min(pitches) % 12
    best: Optional[Tuple[int, str]] = None
    for root in sorted(pcs, key=lambda r: (r != bass, r)):
        rel = {(p - root) % 12 for p in pcs}
        for suffix, tpl in CHORD_TEMPLATES:
            if rel == tpl:
                name = NOTE_NAMES[root] + suffix
                if root != bass:
                    name += "/" + NOTE_NAMES[bass]
                score = (0 if root == bass else 1)
                if best is None or score < best[0]:
                    best = (score, name)
    if best:
        return best[1]
    return "{" + " ".join(NOTE_NAMES[p] for p in sorted(pcs)) + "}"


def chord_segments(trk: Optional[MidiTrack], ppq: int) -> List[Tuple[int, int, str]]:
    """Group the chord track into (start, end, name) by onset."""
    if trk is None or not trk.notes:
        return []
    window = max(1, ppq // 16)   # strums arrive a little spread out
    groups: List[List[MidiNote]] = []
    for n in trk.notes:
        if groups and n.start - groups[-1][0].start <= window:
            groups[-1].append(n)
        else:
            groups.append([n])
    segs: List[Tuple[int, int, str]] = []
    for g in groups:
        name = name_chord([n.pitch for n in g])
        start, end = g[0].start, max(n.end for n in g)
        if segs and segs[-1][2] == name and start <= segs[-1][1] + window:
            segs[-1] = (segs[-1][0], max(end, segs[-1][1]), name)
        else:
            segs.append((start, end, name))
    # a chord lasts until the next one starts
    out = []
    for k, (s, e, nm) in enumerate(segs):
        nxt = segs[k + 1][0] if k + 1 < len(segs) else e
        out.append((s, max(e, nxt), nm))
    return out


def chord_at(segs: List[Tuple[int, int, str]], tick: int) -> Optional[str]:
    cur = None
    for s, e, nm in segs:
        if s <= tick:
            cur = (s, e, nm)
        else:
            break
    if cur and tick < cur[1]:
        return cur[2]
    return None


# ===========================================================================
# Song Cage JSON (layout unknown, so read it loosely)
# ===========================================================================
@dataclass
class Syllable:
    order: int
    text: str
    word: Optional[str]
    section: Optional[str]
    position: Optional[float]
    duration: Optional[float]
    pitches: List[int]
    path: str
    bar_beat: Optional[Tuple[float, float]] = None


@dataclass
class ChordRec:
    name: str
    position: Optional[float]
    section: Optional[str]
    path: str


def _match(key: str, names: List[str]) -> bool:
    k = key.lower().replace("_", "").replace("-", "")
    return k in names


def _find(d: Dict[str, Any], names: List[str], kind: type):
    for k, v in d.items():
        if _match(k, names) and isinstance(v, kind) and not isinstance(v, bool):
            return k, v
    return None, None


def _num(d: Dict[str, Any], names: List[str]) -> Optional[float]:
    for k, v in d.items():
        if _match(k, names):
            if isinstance(v, (int, float)) and not isinstance(v, bool):
                return float(v)
            if isinstance(v, dict):   # e.g. {"bar": 2, "beat": 1.5}
                bar, beat = v.get("bar"), v.get("beat")
                if isinstance(bar, (int, float)) and isinstance(beat, (int, float)):
                    return None  # handled by _bar_beat
    return None


def _bar_beat(d: Dict[str, Any]) -> Optional[Tuple[float, float]]:
    """Return (bar, beat) if the record (or its position dict) carries both."""
    cands = [d] + [v for k, v in d.items() if isinstance(v, dict) and _match(k, KEYMAP["position"])]
    for c in cands:
        bar = next((v for k, v in c.items() if k.lower() in ("bar", "measure")), None)
        beat = next((v for k, v in c.items() if k.lower() in ("beat", "beatinbar")), None)
        if isinstance(bar, (int, float)) and isinstance(beat, (int, float)):
            return float(bar), float(beat)
    return None


def _pitch_of(v: Any) -> Optional[int]:
    if isinstance(v, bool):
        return None
    if isinstance(v, int) and 0 <= v <= 127:
        return v
    if isinstance(v, float) and v.is_integer() and 0 <= v <= 127:
        return int(v)
    if isinstance(v, str):
        return _parse_note_name(v)
    if isinstance(v, dict):
        _k, p = _find(v, KEYMAP["pitch"], (int, float, str))
        return _pitch_of(p) if p is not None else None
    return None


def _parse_note_name(s: str) -> Optional[int]:
    s = s.strip()
    names = {"C": 0, "D": 2, "E": 4, "F": 5, "G": 7, "A": 9, "B": 11}
    if not s or s[0].upper() not in names:
        return None
    pc, i = names[s[0].upper()], 1
    while i < len(s) and s[i] in "#b♯♭":
        pc += 1 if s[i] in "#♯" else -1
        i += 1
    try:
        octave = int(s[i:])
    except ValueError:
        return None
    p = (octave + 1) * 12 + pc
    return p if 0 <= p <= 127 else None


class SongCageDoc:
    def __init__(self, data: Any):
        self.data = data
        self.syllables: List[Syllable] = []
        self.chords: List[ChordRec] = []
        self.meta: Dict[str, Any] = {}
        self.plain_lyrics: List[Tuple[str, Optional[str]]] = []
        self._walk(data, [], None, None)
        self._drop_containers()

    # -- walking ------------------------------------------------------------
    def _walk(self, node: Any, path: List[str], section: Optional[str],
              word: Optional[str]) -> None:
        if isinstance(node, list):
            for idx, item in enumerate(node):
                self._walk(item, path + [f"[{idx}]"], section, word)
            return
        if not isinstance(node, dict):
            return
        keys_above = [p.lower() for p in path if not p.startswith("[")]
        if not path:
            for k in ("title", "name", "tempo", "bpm", "key", "timeSignature",
                      "time_signature", "version", "schemaVersion", "format"):
                if k in node and not isinstance(node[k], (dict, list)):
                    self.meta[k] = node[k]
        if any(any(w in k for w in KEYMAP["section_ancestor"]) for k in keys_above[-1:]):
            _k, nm = _find(node, KEYMAP["section_name"], str)
            if nm:
                section = nm
        lyric_ctx = any(any(w in k for w in KEYMAP["lyric_ancestor"]) for k in keys_above)
        chord_ctx = any(any(w in k for w in KEYMAP["chord_ancestor"]) for k in keys_above[-2:])
        pstr = "/".join(path) or "/"

        if chord_ctx and not lyric_ctx:
            rec = self._chord_from(node, section, pstr)
            if rec:
                self.chords.append(rec)
        if lyric_ctx:
            tkey, text = _find(node, KEYMAP["text"], str)
            if text is not None and text.strip():
                syl = self._syllable_from(node, text, word, section, pstr)
                if syl:
                    self.syllables.append(syl)
                    word_for_children = text
                    # a word whose syllables hang below it: children carry the word
                    for k, v in node.items():
                        if k != tkey and isinstance(v, (dict, list)):
                            self._walk(v, path + [k], section, word_for_children)
                    return
        # plain lyric strings, e.g. "lyrics": "line one\nline two"
        for k, v in node.items():
            if isinstance(v, str) and any(w in k.lower() for w in ("lyric",)) and len(v) > 20:
                self.plain_lyrics.append((v, section))
        for k, v in node.items():
            if isinstance(v, (dict, list)):
                self._walk(v, path + [k], section, word)

    def _syllable_from(self, node, text, word, section, pstr) -> Optional[Syllable]:
        pos = _num(node, KEYMAP["position"])
        dur = _num(node, KEYMAP["duration"])
        pitches: List[int] = []
        _k, p = _find(node, KEYMAP["pitch"], (int, float, str))
        if p is not None and _pitch_of(p) is not None:
            pitches.append(_pitch_of(p))  # type: ignore[arg-type]
        _k, notes = _find(node, KEYMAP["notes"], list)
        if notes:
            for n in notes:
                pp = _pitch_of(n)
                if pp is not None:
                    pitches.append(pp)
        bb = _bar_beat(node)
        syl = Syllable(len(self.syllables), text.strip(), word, section, pos, dur, pitches, pstr, bb)
        return syl

    def _chord_from(self, node, section, pstr) -> Optional[ChordRec]:
        _k, nm = _find(node, KEYMAP["chord_name"], str)
        if not nm:
            _k, root = _find(node, KEYMAP["chord_root"], str)
            if root:
                _k, q = _find(node, KEYMAP["chord_quality"], str)
                nm = root + (q or "")
        if not nm:
            return None
        return ChordRec(nm, _num(node, KEYMAP["position"]), section, pstr)

    def _drop_containers(self) -> None:
        """If a word record has syllable records under it, keep the syllables."""
        paths = [s.path for s in self.syllables]
        keep = []
        for s in self.syllables:
            if any(p != s.path and p.startswith(s.path + "/") for p in paths):
                continue
            keep.append(s)
        for i, s in enumerate(keep):
            s.order = i
        self.syllables = keep


def schema_outline(node: Any, path: str = "", out: Optional[List[str]] = None,
                   depth: int = 0, max_depth: int = 12) -> List[str]:
    """One line per distinct key path, with type and a sample value."""
    if out is None:
        out = []
    seen = {line.split("  ")[0] for line in out}
    if depth > max_depth:
        return out
    if isinstance(node, dict):
        for k, v in node.items():
            p = f"{path}.{k}" if path else k
            if p not in seen:
                out.append(f"{p}  {_describe(v)}")
            schema_outline(v, p, out, depth + 1, max_depth)
    elif isinstance(node, list):
        p = path + "[]"
        for item in node[:3]:
            schema_outline(item, p, out, depth + 1, max_depth)
    return out


def _describe(v: Any) -> str:
    if isinstance(v, dict):
        return f"object({len(v)} keys)"
    if isinstance(v, list):
        return f"list({len(v)})"
    s = json.dumps(v, ensure_ascii=False)
    return f"{type(v).__name__} = {s[:40]}"


# ===========================================================================
# Alignment
# ===========================================================================
@dataclass
class Row:
    syllable: Optional[Syllable]
    note: Optional[MidiNote]
    melisma_of: Optional[int] = None   # syllable order this extra note belongs to
    flags: List[str] = field(default_factory=list)


def align_by_order(syls: List[Syllable], notes: List[MidiNote]) -> List[Row]:
    """Needleman-Wunsch over syllable 'slots' (one per expected note)."""
    slots: List[Tuple[int, Optional[int]]] = []   # (syllable index, expected pitch)
    for si, s in enumerate(syls):
        if s.pitches:
            for p in s.pitches:
                slots.append((si, p))
        else:
            slots.append((si, None))
    n, m = len(slots), len(notes)
    INF = float("inf")
    # cost[i][j]: aligning first i slots with first j notes
    cost = [[INF] * (m + 1) for _ in range(n + 1)]
    back = [[None] * (m + 1) for _ in range(n + 1)]
    cost[0][0] = 0
    for i in range(n + 1):
        for j in range(m + 1):
            c = cost[i][j]
            if c == INF:
                continue
            if i < n and j < m:
                exp = slots[i][1]
                mc = 0.0 if exp is None or exp == notes[j].pitch else 0.6
                if c + mc < cost[i + 1][j + 1]:
                    cost[i + 1][j + 1], back[i + 1][j + 1] = c + mc, "M"
            if i < n and c + 1.0 < cost[i + 1][j]:
                cost[i + 1][j], back[i + 1][j] = c + 1.0, "S"   # slot with no note
            if j < m:
                # an extra note right after a slot is a cheap melisma
                extra = 0.4 if i > 0 else 1.0
                if c + extra < cost[i][j + 1]:
                    cost[i][j + 1], back[i][j + 1] = c + extra, "N"
    ops = []
    i, j = n, m
    while i > 0 or j > 0:
        op = back[i][j]
        ops.append((op, i, j))
        if op == "M":
            i, j = i - 1, j - 1
        elif op == "S":
            i -= 1
        else:
            j -= 1
    ops.reverse()
    rows: List[Row] = []
    first_note_of: Dict[int, bool] = {}
    last_si: Optional[int] = None
    for op, i, j in ops:
        if op == "M":
            si, exp = slots[i - 1]
            note = notes[j - 1]
            s = syls[si]
            if first_note_of.get(si):
                rows.append(Row(None, note, melisma_of=si, flags=["melisma"]))
            else:
                r = Row(s, note)
                if exp is not None and exp != note.pitch:
                    r.flags.append(f"pitch-differs(file={note_name(exp)})")
                rows.append(r)
                first_note_of[si] = True
            last_si = si
        elif op == "S":
            si, _exp = slots[i - 1]
            if not first_note_of.get(si):
                # only report once per syllable, and only if it got no note at all
                if all(not (r.syllable is syls[si]) for r in rows):
                    rows.append(Row(syls[si], None, flags=["no-note"]))
            last_si = si
        else:
            note = notes[j - 1]
            if last_si is not None and first_note_of.get(last_si):
                rows.append(Row(None, note, melisma_of=last_si, flags=["melisma?"]))
            else:
                rows.append(Row(None, note, flags=["no-lyric"]))
    # a syllable that lost every slot to "S" but later got a note is fine;
    # drop duplicate no-note rows for syllables that ended up matched
    matched = {id(r.syllable) for r in rows if r.syllable and r.note}
    return [r for r in rows if not (r.note is None and id(r.syllable) in matched)]


def align_by_position(syls: List[Syllable], notes: List[MidiNote], mf: MidiFile,
                      offset_beats: float = 0.0) -> Optional[Tuple[List[Row], str]]:
    """Match syllables to notes by start time, trying likely units."""
    if sum(1 for s in syls if s.bar_beat) >= max(2, len(syls) // 2):
        syls = [Syllable(s.order, s.text, s.word, s.section,
                         mf.tick_of(*s.bar_beat) if s.bar_beat else None,
                         s.duration, s.pitches, s.path, s.bar_beat) for s in syls]
        only_ticks = True
    else:
        only_ticks = False
    positioned = [s for s in syls if s.position is not None]
    if len(positioned) < max(2, len(syls) // 2) or not notes:
        return None
    ppq = mf.ppq
    first_note = notes[0].start
    units = {
        "quarter-note beats": lambda x: x * ppq,
        "eighth-note beats": lambda x: x * ppq / 2,
        "sixteenths": lambda x: x * ppq / 4,
        "bars of 4/4": lambda x: x * ppq * 4,
        f"ticks@{ppq}": lambda x: x,
        "ticks@960": lambda x: x * ppq / 960,
        "ticks@480": lambda x: x * ppq / 480,
        "seconds": None,
    }
    if only_ticks:
        units = {"bar:beat": lambda x: x}
    tol = max(1, ppq // 24)
    best = None
    for uname, conv in units.items():
        for anchor in ("zero", "first-note", "offset"):
            def to_tick(x: float, conv=conv) -> float:
                return _seconds_to_tick(mf, x) if conv is None else conv(x)
            if anchor == "zero":
                shift = 0.0
            elif anchor == "first-note":
                shift = first_note - to_tick(positioned[0].position)  # type: ignore[arg-type]
            else:
                shift = offset_beats * ppq
            hits = 0
            for s in positioned:
                t = to_tick(s.position) + shift  # type: ignore[arg-type]
                if any(abs(nt.start - t) <= tol for nt in notes):
                    hits += 1
            if best is None or hits > best[0]:
                best = (hits, uname, to_tick, shift)
    if best is None or best[0] < len(positioned) * 0.6:
        return None
    hits, uname, to_tick, shift = best
    rows: List[Row] = []
    used = set()
    claimed_by: Dict[int, int] = {}
    for s in syls:
        if s.position is None:
            rows.append(Row(s, None, flags=["no-position"]))
            continue
        t = to_tick(s.position) + shift
        cand = [k for k, nt in enumerate(notes) if abs(nt.start - t) <= tol and k not in used]
        if cand:
            k = min(cand, key=lambda k: abs(notes[k].start - t))
            used.add(k)
            claimed_by[k] = s.order
            r = Row(s, notes[k])
            if s.pitches and s.pitches[0] != notes[k].pitch:
                r.flags.append(f"pitch-differs(file={note_name(s.pitches[0])})")
            rows.append(r)
        else:
            rows.append(Row(s, None, flags=["no-note"]))
    # leftover notes: melisma if inside a syllable's span, else no-lyric
    order_of_note = sorted(claimed_by.items())
    for k, nt in enumerate(notes):
        if k in used:
            continue
        owner = None
        for ck, so in order_of_note:
            if notes[ck].start <= nt.start:
                owner = so
        if owner is not None:
            rows.append(Row(None, nt, melisma_of=owner, flags=["melisma?"]))
        else:
            rows.append(Row(None, nt, flags=["no-lyric"]))
    rows.sort(key=lambda r: (r.note.start if r.note else _row_tick(r, to_tick, shift)))
    desc = f"by position ({hits}/{len(positioned)} start times match, unit={uname})"
    return rows, desc


def _row_tick(r: Row, to_tick, shift) -> float:
    if r.syllable and r.syllable.position is not None:
        return to_tick(r.syllable.position) + shift
    return float("inf")


def _seconds_to_tick(mf: MidiFile, sec: float) -> float:
    lo, hi = 0, max((n.end for t in mf.tracks for n in t.notes), default=0) + mf.ppq * 64
    for _ in range(40):
        mid = (lo + hi) / 2
        if mf.seconds(int(mid)) < sec:
            lo = mid
        else:
            hi = mid
    return lo


def melody_notes(trk: Optional[MidiTrack]) -> List[MidiNote]:
    if trk is None:
        return []
    out: List[MidiNote] = []
    for n in trk.notes:
        if out and n.start == out[-1].start:   # keep the top note of a stack
            if n.pitch > out[-1].pitch:
                out[-1] = n
            continue
        out.append(n)
    return out


def syllables_from_midi_lyrics(trk: Optional[MidiTrack]) -> List[Syllable]:
    if trk is None:
        return []
    out = []
    for i, (tick, txt) in enumerate(trk.lyrics):
        txt = txt.strip()
        if txt in ("", "\r", "\n", "/", "\\"):
            continue
        out.append(Syllable(len(out), txt.strip("-"), None, None, float(tick), None, [], f"midi-lyric@{tick}"))
    return out


# ===========================================================================
# Report
# ===========================================================================
def report(doc: Optional[SongCageDoc], mf: Optional[MidiFile], args) -> int:
    w = sys.stdout.write
    syls: List[Syllable] = doc.syllables if doc else []
    mel_trk = chord_trk = None
    notes: List[MidiNote] = []
    segs: List[Tuple[int, int, str]] = []
    if mf:
        mel_trk, chord_trk = pick_tracks(mf, args.melody_track, args.chord_track)
        notes = melody_notes(mel_trk)
        segs = chord_segments(chord_trk, mf.ppq)

    w("== Song Cage reader ==\n")
    if doc:
        w(f"Song file: {args.song}\n")
        if doc.meta:
            w("  top-level: " + ", ".join(f"{k}={v}" for k, v in doc.meta.items()) + "\n")
        w(f"  lyric syllables found: {len(syls)}"
          f" (with position: {sum(1 for s in syls if s.position is not None)},"
          f" with pitch: {sum(1 for s in syls if s.pitches)})\n")
        w(f"  chords found: {len(doc.chords)}"
          f" (with position: {sum(1 for c in doc.chords if c.position is not None)})\n")
        if doc.chords:
            names = [c.name for c in doc.chords]
            w("  chord symbols in file: " + " ".join(names[:24]) + (" ..." if len(names) > 24 else "") + "\n")
        sections = []
        for s in syls:
            if s.section and s.section not in sections:
                sections.append(s.section)
        if sections:
            w("  sections: " + ", ".join(sections) + "\n")
        if syls:
            w(f"  first syllable at: {syls[0].path}\n")
        if doc.plain_lyrics and not syls:
            w("  only plain lyric text found (no per-syllable records)\n")
    if mf:
        w(f"MIDI: {args.midi}  ppq={mf.ppq}")
        bpm = mf.tempo_bpm()
        if bpm:
            w(f"  tempo={bpm:.2f}bpm")
        if mf.timesigs:
            w("  time=" + ", ".join(f"{n}/{d}@bar{mf.bar_beat(t)[0]}" for t, n, d in mf.timesigs))
        w("\n")
        for t in mf.tracks:
            role = "melody" if t is mel_trk else "chords" if t is chord_trk else ""
            w(f"  track {t.index}: name={t.name!r} programs={t.programs[:3]} notes={len(t.notes)}"
              f" lyric-events={len(t.lyrics)} {('<- ' + role) if role else ''}\n")

    if not syls and mf:
        midi_syls = syllables_from_midi_lyrics(mel_trk) or [
            s for t in mf.tracks for s in syllables_from_midi_lyrics(t)]
        if midi_syls:
            w("  (using lyric events found inside the MIDI file)\n")
            syls = midi_syls

    if not syls:
        w("\nNo lyric syllables found. Run with --schema and adjust KEYMAP.\n")
        return 2

    # --- align -------------------------------------------------------------
    if mf and notes:
        res = None if args.by_order else align_by_position(syls, notes, mf, args.offset_beats)
        if res:
            rows, how = res
        else:
            rows, how = align_by_order(syls, notes), "by order (syllables in file order vs melody notes in time order)"
    else:
        rows = [Row(s, None, flags=["no-midi"]) for s in syls]
        how = "no MIDI melody, showing the song file alone"
    w(f"Alignment: {how}\n\n")

    # --- table -------------------------------------------------------------
    hdr = f"{'#':>4}  {'section':<12} {'word':<14} {'syllable':<10} {'note':<5} {'bar:beat':>9} {'sec':>7} {'beats':>5}  {'chord':<9} flags\n"
    w(hdr)
    w("-" * (len(hdr) + 10) + "\n")
    out_records = []
    for r in rows:
        s, n = r.syllable, r.note
        if s is None and r.melisma_of is not None:
            owner = syls[r.melisma_of]
            label, sec_name, word = "  ~", owner.section or "", ""
            sylt = f"({owner.text})"
        elif s is None:
            label, sec_name, word, sylt = "  -", "", "", ""
        else:
            label, sec_name, word, sylt = str(s.order + 1), s.section or "", s.word or "", s.text
        if n and mf:
            bar, beat = mf.bar_beat(n.start)
            bb = f"{bar}:{beat:.2f}".rstrip("0").rstrip(".")
            secs = f"{mf.seconds(n.start):7.2f}"
            beats = f"{(n.end - n.start) / mf.ppq:5.2f}"
            chord = None
            if doc and doc.chords:
                chord = None  # JSON chord positions are unit-unknown; MIDI chord track is authoritative
            chord = chord or chord_at(segs, n.start)
            pitch = note_name(n.pitch)
        else:
            bb, secs, beats, pitch = "", "", "", note_name(s.pitches[0]) if s and s.pitches else "-"
            chord = None
        flags = list(r.flags)
        if n and segs and chord is None:
            flags.append("no-chord")
        w(f"{label:>4}  {sec_name[:12]:<12} {word[:14]:<14} {sylt[:10]:<10} {pitch:<5} {bb:>9} {secs:>7} {beats:>5}  {(chord or '-')[:9]:<9} {' '.join(flags)}\n")
        out_records.append({
            "syllable_index": s.order if s else r.melisma_of,
            "text": s.text if s else None,
            "word": s.word if s else None,
            "section": (s.section if s else (syls[r.melisma_of].section if r.melisma_of is not None else None)),
            "continuation": s is None and r.melisma_of is not None,
            "pitch": n.pitch if n else (s.pitches[0] if s and s.pitches else None),
            "start_tick": n.start if n else None,
            "end_tick": n.end if n else None,
            "start_beat": n.start / mf.ppq if (n and mf) else None,
            "length_beats": (n.end - n.start) / mf.ppq if (n and mf) else None,
            "start_sec": round(mf.seconds(n.start), 4) if (n and mf) else None,
            "bar": mf.bar_beat(n.start)[0] if (n and mf) else None,
            "beat_in_bar": round(mf.bar_beat(n.start)[1], 4) if (n and mf) else None,
            "chord": chord,
            "flags": flags,
        })

    # --- summary -----------------------------------------------------------
    total = len(syls)
    matched = sum(1 for r in rows if r.syllable and r.note)
    no_note = sum(1 for r in rows if r.syllable and not r.note)
    no_lyric = sum(1 for r in rows if "no-lyric" in r.flags)
    melisma = sum(1 for r in rows if r.melisma_of is not None)
    differs = sum(1 for r in rows if any(f.startswith("pitch-differs") for f in r.flags))
    no_chord = sum(1 for rec in out_records if "no-chord" in rec["flags"])
    w("\n== Summary ==\n")
    w(f"syllables: {total}   on a note: {matched} ({100 * matched / total:.0f}%)   without a note: {no_note}\n")
    if mf:
        w(f"melody notes: {len(notes)}   extra notes held over a syllable (melisma): {melisma}"
          f"   notes with no lyric: {no_lyric}\n")
        w(f"pitch disagreements between song file and MIDI: {differs}   notes with no chord: {no_chord}\n")
        usable = total and matched / total >= 0.95 and no_lyric <= max(1, len(notes) * 0.05) and differs == 0
        w(f"verdict: {'LINES UP' if usable else 'NEEDS A LOOK'}"
          f" (lines up = 95%+ of syllables on a note, few unlyric'd notes, no pitch disagreements)\n")

    if args.jsonl:
        with open(args.jsonl, "w", encoding="utf-8") as fh:
            for rec in out_records:
                fh.write(json.dumps(rec, ensure_ascii=False) + "\n")
        w(f"wrote {len(out_records)} rows to {args.jsonl}\n")
    return 0


def main(argv: Optional[List[str]] = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("song", nargs="?", help=".songcage.json song file")
    ap.add_argument("midi", nargs="?", help="MIDI exported from the same song")
    ap.add_argument("--midi-only", metavar="MID", help="read a MIDI file without a song file")
    ap.add_argument("--schema", action="store_true", help="print the song file's layout and stop")
    ap.add_argument("--keymap", help="JSON file overriding entries in KEYMAP")
    ap.add_argument("--jsonl", help="write one aligned row per line to this file")
    ap.add_argument("--by-order", action="store_true", help="skip position matching, align by order")
    ap.add_argument("--offset-beats", type=float, default=0.0, help="shift song-file positions (quarter beats)")
    ap.add_argument("--melody-track", type=int, help="force melody track index")
    ap.add_argument("--chord-track", type=int, help="force chord track index")
    args = ap.parse_args(argv)

    if args.keymap:
        with open(args.keymap, encoding="utf-8") as fh:
            KEYMAP.update({k: [x.lower() for x in v] for k, v in json.load(fh).items()})

    doc = None
    if args.midi_only:
        args.midi = args.midi_only
        args.song = None
    elif args.song and args.song.lower().endswith((".mid", ".midi")) and not args.midi:
        args.midi, args.song = args.song, None
    if args.song:
        with open(args.song, encoding="utf-8-sig") as fh:
            data = json.load(fh)
        if args.schema:
            for line in schema_outline(data):
                print(line)
            return 0
        doc = SongCageDoc(data)
    mf = read_midi(args.midi) if args.midi else None
    if not doc and not mf:
        ap.print_help()
        return 1
    return report(doc, mf, args)


if __name__ == "__main__":
    sys.exit(main())
