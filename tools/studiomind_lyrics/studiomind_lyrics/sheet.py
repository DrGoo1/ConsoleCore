"""The lyric sheet: the contract between StudioMind's lyric stage and its
melody/MIDI stage.

A sheet is plain JSON so any stage (Python, Swift on the Resonance side, the
Song Cage reader) can read it without this package.
"""

from __future__ import annotations

import json
from dataclasses import dataclass, field

from . import syllables as syl

SCHEMA_VERSION = "studiomind.lyric_sheet/0.1"

# JSON schema handed to the model as its output format (structured outputs).
# Strict-mode rules: every object lists all its properties as required and
# sets additionalProperties false.
LYRIC_SHEET_SCHEMA: dict = {
    "type": "object",
    "properties": {
        "title": {"type": "string"},
        "seed_phrase": {"type": "string"},
        "theme": {"type": "string", "description": "One sentence on what the song is about."},
        "point_of_view": {"type": "string", "description": "e.g. first person, to a 'you'"},
        "sections": {
            "type": "array",
            "items": {
                "type": "object",
                "properties": {
                    "name": {"type": "string", "description": "e.g. Verse 1, Chorus, Bridge"},
                    "kind": {
                        "type": "string",
                        "enum": ["intro", "verse", "pre_chorus", "chorus", "post_chorus", "bridge", "outro"],
                    },
                    "rhyme_scheme": {"type": "string", "description": "One letter per line, e.g. ABAB; X for unrhymed."},
                    "lines": {
                        "type": "array",
                        "items": {
                            "type": "object",
                            "properties": {
                                "text": {"type": "string"},
                                "syllables": {
                                    "type": "array",
                                    "items": {"type": "string"},
                                    "description": "The line split into sung syllables, in order.",
                                },
                                "stress": {
                                    "type": "array",
                                    "items": {"type": "integer", "enum": [0, 1]},
                                    "description": "1 = stressed, 0 = unstressed; one per syllable.",
                                },
                                "rhyme": {"type": "string", "description": "This line's rhyme letter."},
                            },
                            "required": ["text", "syllables", "stress", "rhyme"],
                            "additionalProperties": False,
                        },
                    },
                },
                "required": ["name", "kind", "rhyme_scheme", "lines"],
                "additionalProperties": False,
            },
        },
    },
    "required": ["title", "seed_phrase", "theme", "point_of_view", "sections"],
    "additionalProperties": False,
}


@dataclass
class LineCheck:
    section: str
    index: int
    text: str
    model_count: int
    heuristic_count: int
    target: int | None
    problems: list[str] = field(default_factory=list)


def check_sheet(sheet: dict, targets: dict[str, list[int]] | None = None, tolerance: int = 1) -> list[LineCheck]:
    """Check every line. Problems are returned, never raised.

    targets maps a section name (or kind) to the syllable count wanted per
    line, e.g. from a melody: {"Chorus": [7, 7, 8, 6]}.
    """
    checks: list[LineCheck] = []
    targets = targets or {}
    for sec in sheet.get("sections", []):
        lines = sec.get("lines", [])
        scheme = sec.get("rhyme_scheme", "")
        sec_targets = targets.get(sec.get("name")) or targets.get(sec.get("kind"))
        for i, line in enumerate(lines):
            parts = line.get("syllables", [])
            stress = line.get("stress", [])
            target = sec_targets[i] if sec_targets and i < len(sec_targets) else None
            c = LineCheck(
                section=sec.get("name", "?"),
                index=i,
                text=line.get("text", ""),
                model_count=len(parts),
                heuristic_count=syl.count_line(line.get("text", "")),
                target=target,
            )
            if len(stress) != len(parts):
                c.problems.append(f"{len(stress)} stress marks for {len(parts)} syllables")
            if abs(c.model_count - c.heuristic_count) > tolerance:
                c.problems.append(
                    f"split has {c.model_count} syllables but the text reads as about {c.heuristic_count}"
                )
            joined = "".join(parts).replace("-", "").lower()
            plain = "".join(syl.words(c.text)).replace("'", "")
            if joined.replace("'", "") and _letters(joined) != _letters(plain):
                c.problems.append("syllable split doesn't spell the line")
            if target is not None and c.model_count != target:
                c.problems.append(f"needs {target} syllables, has {c.model_count}")
            if i == 0 and scheme and len(scheme) != len(lines):
                c.problems.append(f"rhyme scheme {scheme} has {len(scheme)} letters for {len(lines)} lines")
            if scheme and i < len(scheme) and line.get("rhyme") and line["rhyme"] != scheme[i]:
                c.problems.append(f"rhyme letter {line['rhyme']} but scheme says {scheme[i]}")
            checks.append(c)
    return checks


def _letters(s: str) -> str:
    return "".join(ch for ch in s.lower() if ch.isalpha())


def _marked(text: str, parts: list[str], stress: list[int]) -> str:
    """Syllables grouped back into words, stressed ones in capitals:
    "how-EV-er LATE you're COM-ing HOME". Falls back to space-separated
    syllables if the split doesn't line up with the words."""
    shown = [p.upper() if (j < len(stress) and stress[j]) else p.lower() for j, p in enumerate(parts)]
    out, j = [], 0
    for word in syl.words(text):
        need, got, pieces = _letters(word), "", []
        while j < len(parts) and len(got) < len(need):
            got += _letters(parts[j])
            pieces.append(shown[j])
            j += 1
        if got != need:
            return " ".join(shown)
        out.append("-".join(pieces))
    return " ".join(out) if j == len(parts) else " ".join(shown)


def render(sheet: dict, checks: list[LineCheck] | None = None) -> str:
    """Human-readable lyric sheet with syllable counts and stress marks."""
    by_key = {(c.section, c.index): c for c in (checks or [])}
    out: list[str] = []
    out.append(sheet.get("title", "Untitled").upper())
    out.append(f'Seed: "{sheet.get("seed_phrase", "")}"')
    if sheet.get("theme"):
        out.append(f"Theme: {sheet['theme']}")
    out.append("")
    for sec in sheet.get("sections", []):
        scheme = sec.get("rhyme_scheme", "")
        out.append(f"[{sec.get('name', '?')}]  rhyme {scheme}")
        for i, line in enumerate(sec.get("lines", [])):
            parts = line.get("syllables", [])
            stress = line.get("stress", [])
            marked = _marked(line.get("text", ""), parts, stress)
            c = by_key.get((sec.get("name", "?"), i))
            flag = "  !" if c and c.problems else ""
            tgt = f"/{c.target}" if c and c.target is not None else ""
            out.append(f"  {len(parts):>2}{tgt:<3} {line.get('rhyme', ' ')}  {line.get('text', '')}{flag}")
            out.append(f"          {marked}")
        out.append("")
    problems = [c for c in (checks or []) if c.problems]
    if problems:
        out.append("Check these lines:")
        for c in problems:
            out.append(f"  {c.section} line {c.index + 1}: " + "; ".join(c.problems))
    return "\n".join(out).rstrip() + "\n"


def to_melody_handoff(sheet: dict) -> dict:
    """Flatten the sheet into what a melody fitter needs: one entry per sung
    syllable, in order, with its section, line, stress and whether it ends a
    line (a natural place for a long note or a rest)."""
    events = []
    for s_i, sec in enumerate(sheet.get("sections", [])):
        for l_i, line in enumerate(sec.get("lines", [])):
            parts = line.get("syllables", [])
            stress = line.get("stress", [])
            for j, p in enumerate(parts):
                events.append(
                    {
                        "section": sec.get("name"),
                        "section_kind": sec.get("kind"),
                        "section_index": s_i,
                        "line_index": l_i,
                        "syllable_index": j,
                        "syllable": p,
                        "stress": stress[j] if j < len(stress) else 0,
                        "line_end": j == len(parts) - 1,
                    }
                )
    return {"schema": SCHEMA_VERSION + "+syllables", "title": sheet.get("title"), "syllables": events}


def load(path: str) -> dict:
    with open(path, encoding="utf-8") as f:
        return json.load(f)
