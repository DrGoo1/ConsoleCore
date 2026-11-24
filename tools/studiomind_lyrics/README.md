# studiomind-lyrics

Seed phrase in, lyric sheet out: sections, rhyme scheme, and every line split
into syllables with stress marks, ready for the melody stage.
Design: [docs/STUDIOMIND_LYRIC_GENERATOR.md](../../docs/STUDIOMIND_LYRIC_GENERATOR.md).

## Run

```bash
cd tools/studiomind_lyrics
python3 -m pip install -r requirements.txt
export ANTHROPIC_API_KEY=...        # read from the environment only

python3 -m studiomind_lyrics "the porch light stays on"
python3 -m studiomind_lyrics "the porch light stays on" --genre "folk rock" --mood hopeful \
    --targets chorus=8,6,7,6 --json sheet.json --melody-json syllables.json
```

Check and print a saved sheet without calling the API:

```bash
python3 -m studiomind_lyrics --from-json examples/sample_sheet.json --targets chorus=8,6,7,6
```

Options: `--form`, `--pov`, `--notes`, `--examples FILE...` (your own lyrics as
style examples), `--model` (default `claude-opus-5-5`). Exit code is 1 when any
line still fails the syllable gate.

## Output, per line

```
   8/8  A  However late you're coming home
          how-EV-er LATE you're COM-ing HOME
```

Syllable count (and target), rhyme letter, the line, then the sung syllables
with stressed ones in capitals. A `!` marks a line that failed a check.

## Test

```bash
python3 -m pytest -q tests
```

The tests use a stand-in client, so they need no API key.
