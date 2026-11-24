"""Tests for the key-independent string layering renderer. Run: python3 -m unittest test_string_layer"""
import tempfile
import unittest
from pathlib import Path

import string_layer as sl

RULES = sl.load_rules()
KEYS = ["C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"]


def notes_by(tracks):
    return {t.name: t.notes for t in tracks}


class StringLayerTests(unittest.TestCase):
    def test_capo_sum_matches_key_in_every_key(self):
        for key in KEYS:
            capos = sl.choose_capos(RULES, sl.KEY_PC[key])
            for role, (shape, capo) in capos.items():
                self.assertEqual((RULES["shape_keys"][shape]["offset"] + capo) % 12, sl.KEY_PC[key], (key, role))
            self.assertLessEqual(capos["main"][1], 7)
            self.assertTrue(4 <= capos["high"][1] <= 9, key)
            self.assertNotEqual(capos["main"][0], capos["high"][0])

    def test_chord_following_parts_only_play_chord_tones(self):
        follow = ["Bass", "Acoustic drive (16ths)", "Mandolin (verse 16ths)", "Acoustic pair L (open)",
                  "Acoustic pair R (open)", "Acoustic capo (high voicing)", "Acoustic Nashville tuning",
                  "Banjo roll (6-string)", "Electric power chords L"]
        for key in ["G", "Bb", "E"]:
            kpc = sl.KEY_PC[key]
            tracks, _, _ = sl.render(RULES, key)
            for tname, notes in notes_by(tracks).items():
                if tname not in follow:
                    continue
                for n in notes:
                    if n.get("approach_to"):
                        continue
                    roman = n["chord"]
                    root = sl.chord_root_pc(kpc, roman)
                    pcs = {(root + iv) % 12 for iv in sl.TONE[sl.ROMAN[roman][1]].values()}
                    self.assertIn(n["midi"] % 12, pcs, (key, tname, n))

    def test_anchor_pedal_and_static_mandolin_never_change_notes(self):
        tracks, _, _ = sl.render(RULES, "G")
        by = notes_by(tracks)
        self.assertEqual({n["midi"] for n in by["Acoustic anchor (repeats)"]}, {55, 62, 69, 74})
        self.assertEqual({n["midi"] for n in by["Electric pedal (8ths)"]}, {67})
        self.assertEqual({n["midi"] for n in by["Mandolin (one chord)"]}, {67, 71, 74, 79})

    def test_key_change_transposes_key_relative_parts(self):
        g = notes_by(sl.render(RULES, "G")[0])
        bb = notes_by(sl.render(RULES, "Bb")[0])
        for tname in ["Acoustic anchor (repeats)", "Electric pedal (8ths)", "Mandolin (one chord)"]:
            self.assertEqual(sorted({n["midi"] + 3 for n in g[tname]}), sorted({n["midi"] for n in bb[tname]}), tname)
        # main pair: G-shape capo 0 in G, G-shape capo 3 in Bb -> exactly 3 semitones up
        self.assertEqual([n["midi"] + 3 for n in g["Acoustic pair L (open)"]][:50],
                         [n["midi"] for n in bb["Acoustic pair L (open)"]][:50])

    def test_sections_follow_the_video(self):
        tracks, _, bars = sl.render(RULES, "G")
        by = notes_by(tracks)
        sections = lambda t: {n["section"] for n in by[t]}
        self.assertEqual(sections("Electric power chords L"), {"chorus1", "chorus2", "bridge1"})
        self.assertEqual(sections("Electric pedal (8ths)"), {"verse2"})
        self.assertEqual(sections("Acoustic drive (16ths)"), {"verse1", "verse2"})
        bridge_capo = [n for n in by["Acoustic capo (high voicing)"] if n["section"] == "bridge1"]
        self.assertTrue(all(n["dur"] > sl.BAR * 0.9 for n in bridge_capo))  # whole notes
        bridge_pair = [n for n in by["Acoustic pair L (open)"] if n["section"] == "bridge1"]
        self.assertGreater(len({n["step"] for n in bridge_pair}), 1)  # keeps playing its pattern

    def test_writes_valid_midi(self):
        tracks, _, _ = sl.render(RULES, "A")
        with tempfile.TemporaryDirectory() as d:
            out = Path(d) / "t.mid"
            sl.write_midi(out, tracks, 110, "test")
            data = out.read_bytes()
        self.assertEqual(data[:4], b"MThd")
        self.assertEqual(data.count(b"MTrk"), len(tracks) + 1)


if __name__ == "__main__":
    unittest.main()
