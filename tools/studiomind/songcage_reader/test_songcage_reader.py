"""Tests for songcage_reader using made-up song files.

The real .songcage.json layout is unpublished, so these use two guessed
layouts to check the reader copes with different shapes.
Run: python3 -m unittest test_songcage_reader -v   (from this folder)
"""
import io
import json
import os
import struct
import sys
import tempfile
import unittest
from contextlib import redirect_stdout

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import songcage_reader as scr  # noqa: E402

PPQ = 480


def varlen(v):
    out = [v & 0x7F]
    v >>= 7
    while v:
        out.append((v & 0x7F) | 0x80)
        v >>= 7
    return bytes(reversed(out))


def track(events):
    """events: list of (tick, bytes) -> MTrk chunk"""
    events = sorted(events, key=lambda e: e[0])
    body, last = b"", 0
    for tick, data in events:
        body += varlen(tick - last) + data
        last = tick
    body += varlen(0) + b"\xff\x2f\x00"
    return b"MTrk" + struct.pack(">I", len(body)) + body


def meta(kind, payload):
    return b"\xff" + bytes([kind]) + varlen(len(payload)) + payload


def write_midi(path, melody, chords, bpm=90, lyrics=None):
    """melody: [(beat, len, pitch)]  chords: [(beat, len, [pitches])]"""
    t0 = [(0, meta(3, b"Test Song")),
          (0, meta(0x51, int(60e6 / bpm).to_bytes(3, "big"))),
          (0, meta(0x58, bytes([4, 2, 24, 8])))]
    ch = [(0, meta(3, b"Chords")), (0, b"\xc0\x00")]
    for b, ln, ps in chords:
        for p in ps:
            ch.append((int(b * PPQ), bytes([0x90, p, 80])))
            ch.append((int((b + ln) * PPQ) - 1, bytes([0x80, p, 0])))
    me = [(0, meta(3, b"Melody")), (0, b"\xc1\x49")]
    for k, (b, ln, p) in enumerate(melody):
        me.append((int(b * PPQ), bytes([0x91, p, 100])))
        me.append((int((b + ln) * PPQ) - 1, bytes([0x81, p, 0])))
        if lyrics and k < len(lyrics) and lyrics[k]:
            me.append((int(b * PPQ), meta(5, lyrics[k].encode())))
    data = b"MThd" + struct.pack(">IHHH", 6, 1, 3, PPQ) + track(t0) + track(ch) + track(me)
    with open(path, "wb") as fh:
        fh.write(data)


# "Hel-lo dar-ling" then a held "oh" over two notes (melisma)
MELODY = [(0, 1, 64), (1, 1, 62), (2, 1, 60), (3, 1, 62), (4, 1, 64), (5, 2, 67)]
CHORDS = [(0, 4, [48, 52, 55]), (4, 4, [53, 57, 60])]   # C then F


def run(argv):
    buf = io.StringIO()
    with redirect_stdout(buf):
        code = scr.main(argv)
    return code, buf.getvalue()


class ReaderTests(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp()
        self.mid = os.path.join(self.dir, "song.mid")
        write_midi(self.mid, MELODY, CHORDS)

    def _json(self, obj):
        p = os.path.join(self.dir, "song.songcage.json")
        with open(p, "w") as fh:
            json.dump(obj, fh)
        return p

    def test_layout_words_with_syllables_and_beats(self):
        song = {"title": "Test Song", "tempo": 90, "sections": [{
            "name": "Verse 1",
            "lyrics": [
                {"text": "hello", "syllables": [
                    {"text": "hel", "start": 0, "notes": [{"pitch": 64}]},
                    {"text": "lo", "start": 1, "notes": [{"pitch": 62}]}]},
                {"text": "darling", "syllables": [
                    {"text": "dar", "start": 2, "notes": [{"pitch": 60}]},
                    {"text": "ling", "start": 3, "notes": [{"pitch": 62}]}]},
                {"text": "oh", "start": 4, "notes": [{"pitch": 64}, {"pitch": 67}]},
            ],
            "chords": [{"symbol": "C", "start": 0}, {"symbol": "F", "start": 4}],
        }]}
        p = self._json(song)
        jl = os.path.join(self.dir, "out.jsonl")
        code, out = run([p, self.mid, "--jsonl", jl])
        self.assertEqual(code, 0, out)
        self.assertIn("syllables: 5", out)
        self.assertIn("on a note: 5 (100%)", out)
        rows = [json.loads(l) for l in open(jl)]
        first = rows[0]
        self.assertEqual((first["text"], first["word"], first["pitch"], first["chord"]),
                         ("hel", "hello", 64, "C"))
        oh = [r for r in rows if r["text"] == "oh"][0]
        self.assertEqual(oh["chord"], "F")
        self.assertTrue(any(r["continuation"] for r in rows), "melisma note missing")
        self.assertIn("Verse 1", out)

    def test_layout_flat_blocks_no_positions(self):
        song = {"song": {"parts": [{"label": "Chorus", "blocks": [
            {"word": "hel"}, {"word": "lo"}, {"word": "dar"}, {"word": "ling"}, {"word": "oh"}]}]}}
        p = self._json(song)
        code, out = run([p, self.mid])
        self.assertEqual(code, 0, out)
        self.assertIn("by order", out)
        self.assertIn("on a note: 5 (100%)", out)

    def test_extra_syllable_flagged(self):
        song = {"lyrics": [{"text": t, "pitch": p} for t, p in
                           [("a", 64), ("b", 62), ("c", 60), ("d", 62), ("e", 64), ("f", 67), ("g", 70)]]}
        p = self._json(song)
        code, out = run([p, self.mid, "--by-order"])
        self.assertIn("no-note", out)
        self.assertIn("NEEDS A LOOK", out)

    def test_midi_lyric_events_used_when_no_song_file(self):
        write_midi(self.mid, MELODY, CHORDS, lyrics=["hel", "lo", "dar", "ling", "oh", ""])
        code, out = run([self.mid])
        self.assertEqual(code, 0, out)
        self.assertIn("lyric events found inside the MIDI", out)
        self.assertIn("on a note: 5 (100%)", out)

    def test_schema_outline(self):
        p = self._json({"a": {"b": [{"c": 1}]}})
        code, out = run([p, "--schema"])
        self.assertIn("a.b[].c  int = 1", out)

    def test_chord_names(self):
        self.assertEqual(scr.name_chord([48, 52, 55]), "C")
        self.assertEqual(scr.name_chord([57, 60, 64]), "Am")
        self.assertEqual(scr.name_chord([52, 55, 60]), "C/E")
        self.assertEqual(scr.name_chord([43, 47, 50, 53]), "G7")


if __name__ == "__main__":
    unittest.main()
