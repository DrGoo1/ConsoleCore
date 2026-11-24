"""studiomind-lyrics: seed phrase in, lyric sheet out.

  python -m studiomind_lyrics "the porch light stays on"
  python -m studiomind_lyrics "the porch light stays on" --genre "folk rock" --mood hopeful \\
      --targets chorus=7,7,8,6 --examples ~/Lyrics/mine/*.txt --json out.json
  python -m studiomind_lyrics --from-json examples/sample_sheet.json   # check and print a saved sheet, no API call
"""

from __future__ import annotations

import argparse
import json
import sys

from . import sheet as sheet_mod


def _parse_targets(values: list[str] | None) -> dict[str, list[int]] | None:
    if not values:
        return None
    out: dict[str, list[int]] = {}
    for v in values:
        name, _, counts = v.partition("=")
        if not counts:
            raise SystemExit(f"--targets expects NAME=7,7,8,6 (got {v!r})")
        out[name.strip()] = [int(n) for n in counts.split(",") if n.strip()]
    return out


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(prog="studiomind-lyrics", description="Write a lyric sheet from a Resonance seed phrase.")
    p.add_argument("seed", nargs="?", help="the seed phrase the listener chose")
    p.add_argument("--genre")
    p.add_argument("--mood")
    p.add_argument("--form", help='e.g. "verse chorus verse chorus bridge chorus"')
    p.add_argument("--pov", help="point of view, e.g. 'first person, to a you'")
    p.add_argument("--notes", help="anything else for the brief")
    p.add_argument("--targets", nargs="*", metavar="NAME=N,N,...",
                   help="syllables per line from a melody, by section name or kind")
    p.add_argument("--examples", nargs="*", metavar="FILE",
                   help="text files of your own lyrics, used as style examples")
    p.add_argument("--model", default=None, help="Claude model id (default claude-opus-5-5)")
    p.add_argument("--json", metavar="PATH", help="also write the sheet as JSON")
    p.add_argument("--melody-json", metavar="PATH", help="also write the per-syllable handoff for the melody fitter")
    p.add_argument("--from-json", metavar="PATH", help="check and print a saved sheet instead of calling the model")
    args = p.parse_args(argv)

    targets = _parse_targets(args.targets)

    if args.from_json:
        data = sheet_mod.load(args.from_json)
        checks = sheet_mod.check_sheet(data, targets)
        attempts = None
    else:
        if not args.seed:
            p.error("give a seed phrase, or --from-json PATH")
        from .generate import DEFAULT_MODEL, Brief, generate, has_credentials

        if not has_credentials():
            print("No API credentials found. Set ANTHROPIC_API_KEY in the environment "
                  "(or run `ant auth login`), then try again.", file=sys.stderr)
            return 2
        examples = []
        for path in args.examples or []:
            with open(path, encoding="utf-8") as f:
                examples.append(f.read())
        brief = Brief(seed_phrase=args.seed, genre=args.genre, mood=args.mood, form=args.form,
                      point_of_view=args.pov, notes=args.notes, targets=targets,
                      style_examples=examples or None)
        result = generate(brief, model=args.model or DEFAULT_MODEL)
        data, checks, attempts = result.sheet, result.checks, result.attempts

    sys.stdout.write(sheet_mod.render(data, checks))
    if attempts:
        print(f"\n({attempts} attempt{'s' if attempts > 1 else ''}, {data.get('model', '')})")
    if args.json:
        with open(args.json, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
    if args.melody_json:
        with open(args.melody_json, "w", encoding="utf-8") as f:
            json.dump(sheet_mod.to_melody_handoff(data), f, indent=2, ensure_ascii=False)
    return 1 if any(c.problems for c in checks) else 0


if __name__ == "__main__":
    raise SystemExit(main())
