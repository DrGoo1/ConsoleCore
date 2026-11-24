# StudioMind lyric generator (Resonance stage 1)

Status: prototype, Nov 24 2026. Code: `tools/studiomind_lyrics/`.

## What it does

Resonance gives the listener three seed phrases; they pick one. The lyric
generator turns that phrase into a **lyric sheet**: a complete song lyric
broken into sections and lines, with a rhyme scheme, and every line split
into sung syllables with stress marks. The melody/MIDI side reads the sheet
and places one note (or more) per syllable.

```
seed phrase + brief ──► Claude (structured JSON) ──► syllable gate ──► lyric sheet ──► melody fitter
                         ▲                              │ fail (once)
                         └──────── problems listed ◄────┘
```

## Design

1. **Input.** The seed phrase, plus an optional brief: genre, mood, form,
   point of view, notes, and **syllable targets per line** when the song plan
   has already chosen a melody (e.g. `chorus = 8,6,7,6`). Optionally, a few of
   Dave's own lyrics as style examples.
2. **Output.** JSON (`studiomind.lyric_sheet/0.1`): title, seed, theme, point
   of view, and sections (`verse`, `chorus`, `bridge`, ...), each with a rhyme
   scheme (`ABAB`) and lines of `{text, syllables[], stress[], rhyme}`.
   `to_melody_handoff()` flattens it to one event per syllable with section,
   line, stress and a `line_end` flag (a natural place for a long note or rest).
3. **Writer.** The Claude API with the latest Claude model (`claude-opus-5-5`),
   prompted, not trained, using structured outputs so the result always parses.
   This matches the songwriting plan: "a language model, prompted with your
   briefs, not trained", because no lyric corpus Dave owns would make a small
   trained model beat a large one.
4. **Gate.** Plain code checks every line: the syllable split must spell the
   line, its count must agree with an independent syllable counter (±1), it
   must hit any melody target exactly, and stress and rhyme marks must line up.
   Failing lines go back to the model once with the problems listed; anything
   still wrong is flagged on the sheet for a human pick or edit.
5. **Melody side.** Two directions, both using the same sheet:
   - *Lyrics first* (no melody yet): the sheet's counts and stresses become
     constraints for the song plan's melody builder (strong beats on stressed
     syllables, phrase length from line length).
   - *Melody first* (song plan or a Song Cage file already has a melody): pass
     its per-line note counts as `--targets`, so the lyric is written to fit.
   The Song Cage reader being built in a parallel task does the note-to-syllable
   alignment; this sheet is meant to be its lyric input, not a second aligner.

## Licensing

- **No training on anyone's lyrics.** The model is used as is. Style examples
  in the prompt are limited to lyrics Dave wrote; the prompt tells the model
  not to quote or imitate existing songs or artists.
- **Ownership of output.** Under Anthropic's commercial API terms, the
  customer owns the outputs. Worth confirming against the terms on the account
  that holds the key.
- **Copyright needs a human hand.** The US Copyright Office says prompting
  alone doesn't make a work copyrightable. Picking among drafts and editing
  lines, which the pipeline expects Dave to do, is what makes the song his.
- **Later option:** a small model fine-tuned on Dave's own catalogue for
  voice. Only worth it if blind tests show the prompted lyrics don't sound
  like him; it would train only on songs he owns.

## Where the code should live

The prototype sits in ConsoleCore because that's the repository connected to
the StudioMind project. It is a self-contained Python package with one
dependency (`anthropic`), so it can move to `StudioMind_AI/resonance/` as is.
That needs the StudioMind_AI repository added to the project.

## Not done yet

- A live run against the API (no key in the cloud session that built this).
- Resonance's iPhone side calling it (it would call a small StudioMind
  service, never the API directly from the phone, so the key stays off devices).
- Rhyme checking by sound; today only the rhyme letters are checked for
  consistency.
- Three drafts per seed for Dave to pick from (milestone 1 of the plan asks for
  this; it's a loop around `generate()`).
