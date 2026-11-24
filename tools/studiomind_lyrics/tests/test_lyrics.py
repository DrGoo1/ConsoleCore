import copy
import json
import os
import sys
from types import SimpleNamespace

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

from studiomind_lyrics import sheet as sheet_mod  # noqa: E402
from studiomind_lyrics.generate import Brief, build_user_prompt, generate  # noqa: E402
from studiomind_lyrics.syllables import count_line, count_word  # noqa: E402

SAMPLE = os.path.join(os.path.dirname(__file__), "..", "examples", "sample_sheet.json")


def test_common_words():
    cases = {"love": 1, "time": 1, "table": 2, "walked": 1, "waited": 2, "morning": 2,
             "beautiful": 3, "remember": 3, "lonely": 2, "someday": 2, "fire": 1,
             "everything": 3, "radio": 3, "driveway": 2, "boxes": 2, "times": 1}
    wrong = {w: count_word(w) for w, n in cases.items() if count_word(w) != n}
    assert not wrong, wrong


def test_line():
    assert count_line("The porch light stays on") == 5
    assert count_line("However late you're coming home") == 8


def test_sample_sheet_passes_with_targets():
    data = sheet_mod.load(SAMPLE)
    checks = sheet_mod.check_sheet(data, {"chorus": [8, 6, 7, 6]})
    assert not [c for c in checks if c.problems]


def test_gate_catches_bad_split_and_target():
    data = sheet_mod.load(SAMPLE)
    line = data["sections"][1]["lines"][1]
    line["syllables"] = ["where", "ev", "er", "you", "have", "gone"]  # extra letter
    checks = sheet_mod.check_sheet(data, {"chorus": [8, 7, 7, 6]})
    probs = [p for c in checks if c.section == "Chorus" and c.index == 1 for p in c.problems]
    assert any("spell" in p for p in probs)
    assert any("needs 7" in p for p in probs)


def test_melody_handoff_counts_every_syllable():
    data = sheet_mod.load(SAMPLE)
    handoff = sheet_mod.to_melody_handoff(data)
    total = sum(len(l["syllables"]) for s in data["sections"] for l in s["lines"])
    assert len(handoff["syllables"]) == total
    assert sum(1 for e in handoff["syllables"] if e["line_end"]) == sum(len(s["lines"]) for s in data["sections"])


def test_prompt_carries_seed_targets_and_examples():
    p = build_user_prompt(Brief("the porch light stays on", targets={"chorus": [7, 7]}, style_examples=["my own line"]))
    assert '"the porch light stays on"' in p and "chorus: 7, 7" in p and "my own line" in p


class FakeClient:
    """Stands in for anthropic.Anthropic: returns a bad sheet, then a good one."""

    def __init__(self, sheets):
        self.sheets = list(sheets)
        self.calls = []
        self.beta = SimpleNamespace(messages=SimpleNamespace(create=self._create))

    def _create(self, **kwargs):
        self.calls.append(copy.deepcopy(kwargs))
        text = json.dumps(self.sheets.pop(0))
        return SimpleNamespace(stop_reason="end_turn", model=kwargs["model"],
                               content=[SimpleNamespace(type="text", text=text)])


def test_generate_retries_once_then_passes():
    good = sheet_mod.load(SAMPLE)
    bad = copy.deepcopy(good)
    bad["sections"][1]["lines"][3]["syllables"].append("now")
    bad["sections"][1]["lines"][3]["stress"].append(0)
    client = FakeClient([bad, good])
    result = generate(Brief("the porch light stays on", targets={"chorus": [8, 6, 7, 6]}), client=client)
    assert result.attempts == 2
    assert not [c for c in result.checks if c.problems]
    retry_msg = client.calls[1]["messages"][-1]["content"]
    assert "Chorus, line 4" in retry_msg
    fmt = client.calls[0]["output_config"]["format"]
    assert fmt["type"] == "json_schema" and fmt["schema"]["required"]
