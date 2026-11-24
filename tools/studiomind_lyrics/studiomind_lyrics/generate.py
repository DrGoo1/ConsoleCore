"""Ask Claude for a lyric sheet, then gate it.

Design choices (see docs/STUDIOMIND_LYRIC_GENERATOR.md):
- Prompted, not trained: a large general model with the brief and, optionally,
  a few of Dave's own lyrics as style examples. Nothing here trains a model.
- Structured output: the model must return JSON matching LYRIC_SHEET_SCHEMA,
  so the melody side never parses free text.
- One gated retry: lines whose syllables don't add up, or miss a melody's
  targets, are sent back once with the problems listed.
"""

from __future__ import annotations

import json
import os
from dataclasses import dataclass

from .sheet import LYRIC_SHEET_SCHEMA, SCHEMA_VERSION, LineCheck, check_sheet

DEFAULT_MODEL = "claude-opus-5-5"

SYSTEM_PROMPT = """You are the lyric stage of StudioMind, a songwriting system that writes songs as MIDI and plays them through real instruments.

You receive a seed phrase the listener chose in the Resonance app, and a short brief. Write one complete, original song lyric that grows out of the seed phrase.

How to write:
- The seed phrase is the emotional centre. Use it, or a close variation, as the chorus hook unless the brief says otherwise.
- Concrete images over abstractions. One clear situation, one point of view, held through the song.
- Every line must be singable: natural word stress, open vowels on long or high notes where you can, no tongue-twisters.
- Keep lines short enough to sing in one breath. Verses tell, the chorus states, the bridge turns.
- Write only original words. Do not quote, paraphrase or imitate a specific existing song or artist.

How to format (the melody stage reads this, so precision matters):
- Split every line into the syllables a singer would sing, in order, with no missing letters ("morn","ing","light"). Joined together they must spell the line's words exactly, ignoring punctuation and spaces.
- Mark each syllable's stress: 1 for stressed, 0 for unstressed, one mark per syllable. Use natural spoken stress.
- Give each section a rhyme scheme with one letter per line (X for an unrhymed line), and give each line its letter.
- When the brief gives syllable targets for a section, every line in that section must have exactly that many syllables."""


@dataclass
class Brief:
    seed_phrase: str
    genre: str | None = None
    mood: str | None = None
    form: str | None = None  # e.g. "verse chorus verse chorus bridge chorus"
    point_of_view: str | None = None
    notes: str | None = None
    # Syllables wanted per line, keyed by section name or kind,
    # e.g. {"chorus": [7, 7, 8, 6]} from a melody the song plan already chose.
    targets: dict[str, list[int]] | None = None
    # Dave's own lyrics as style examples. Only material he owns goes here.
    style_examples: list[str] | None = None


def build_user_prompt(brief: Brief) -> str:
    parts = [f'Seed phrase: "{brief.seed_phrase}"']
    if brief.genre:
        parts.append(f"Genre: {brief.genre}")
    if brief.mood:
        parts.append(f"Mood: {brief.mood}")
    parts.append(f"Form: {brief.form or 'verse, chorus, verse, chorus, bridge, chorus'}")
    if brief.point_of_view:
        parts.append(f"Point of view: {brief.point_of_view}")
    if brief.notes:
        parts.append(f"Notes from the writer: {brief.notes}")
    if brief.targets:
        lines = [f"- {name}: {', '.join(str(n) for n in counts)} syllables per line"
                 for name, counts in brief.targets.items()]
        parts.append("Syllable targets from the melody (exact):\n" + "\n".join(lines))
    else:
        parts.append("No melody yet: keep verse lines around 7 to 9 syllables and chorus lines around 6 to 8, "
                     "with matching counts in lines that will share a melody.")
    if brief.style_examples:
        ex = "\n\n---\n\n".join(e.strip() for e in brief.style_examples)
        parts.append("Lyrics by the same writer, for voice and vocabulary only (don't reuse their lines):\n\n" + ex)
    return "\n\n".join(parts)


def _retry_prompt(checks: list[LineCheck]) -> str:
    bad = [c for c in checks if c.problems]
    lines = [f'- {c.section}, line {c.index + 1} ("{c.text}"): ' + "; ".join(c.problems) for c in bad]
    return ("These lines failed the syllable check. Rewrite only what's needed to fix them, keep everything else, "
            "and return the whole sheet again:\n" + "\n".join(lines))


@dataclass
class Result:
    sheet: dict
    checks: list[LineCheck]
    attempts: int
    model: str


def generate(brief: Brief, model: str = DEFAULT_MODEL, client=None, max_attempts: int = 2, tolerance: int = 1) -> Result:
    """Call Claude for a sheet; retry once if the gate finds problems.

    `client` is an anthropic.Anthropic instance; created from the environment
    (ANTHROPIC_API_KEY or an `ant auth login` profile) when not given.
    """
    if client is None:
        import anthropic  # imported here so --from-json works without the SDK

        client = anthropic.Anthropic()

    messages: list[dict] = [{"role": "user", "content": build_user_prompt(brief)}]
    sheet: dict = {}
    checks: list[LineCheck] = []
    attempts = 0
    for attempts in range(1, max_attempts + 1):
        response = client.beta.messages.create(
            model=model,
            max_tokens=16000,
            system=SYSTEM_PROMPT,
            messages=messages,
            output_config={
                "effort": "high",
                "format": {"type": "json_schema", "schema": LYRIC_SHEET_SCHEMA},
            },
            # If the model declines, let the API re-run on a fallback model.
            betas=["server-side-fallback-2026-07-01"],
            fallbacks="default",
        )
        if response.stop_reason == "refusal":
            raise RuntimeError("The model declined this seed phrase; try another phrase or brief.")
        if response.stop_reason == "max_tokens":
            raise RuntimeError("The response was cut off before the sheet was complete.")
        text = next(b.text for b in response.content if b.type == "text")
        sheet = json.loads(text)
        sheet["schema"] = SCHEMA_VERSION
        sheet["model"] = response.model
        checks = check_sheet(sheet, brief.targets, tolerance)
        if not any(c.problems for c in checks) or attempts == max_attempts:
            break
        # Append-only history: the model's answer, then the gate's feedback.
        messages.append({"role": "assistant", "content": response.content})
        messages.append({"role": "user", "content": _retry_prompt(checks)})
    return Result(sheet=sheet, checks=checks, attempts=attempts, model=sheet.get("model", model))


def has_credentials() -> bool:
    return bool(os.environ.get("ANTHROPIC_API_KEY") or os.environ.get("ANTHROPIC_AUTH_TOKEN")
                or os.environ.get("ANTHROPIC_PROFILE"))
