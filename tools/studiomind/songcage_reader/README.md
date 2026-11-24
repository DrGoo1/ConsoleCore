# Song Cage reader (StudioMind)

Checks whether a song written in Song Cage can become StudioMind training data
with lyrics lined up to melody. It reads the song file (`.songcage.json`) and
the MIDI Song Cage exported for the same song, then prints every syllable next
to its melody note, bar and beat, time in seconds, and the chord under it.

Standard library only (Python 3.8+), so nothing to install.

```
python3 songcage_reader.py "My Song.songcage.json" "My Song.mid"
python3 songcage_reader.py "My Song.songcage.json" "My Song.mid" --jsonl aligned.jsonl
python3 songcage_reader.py "My Song.songcage.json" --schema
```

## What the flags in the table mean

- `no-note`: a syllable that no melody note could be found for.
- `no-lyric`: a melody note with no syllable on it.
- `melisma` / `melisma?`: an extra note sung on the syllable above it
  (`?` means it was inferred from timing rather than stated in the file).
- `pitch-differs(file=X)`: the song file and the MIDI disagree on the note.
- `no-chord`: no chord was sounding under the note.

The summary ends with **LINES UP** (95%+ of syllables on a note, few
unlyric'd notes, no pitch disagreements) or **NEEDS A LOOK**.

## The song file layout is unknown

Song Cage doesn't publish the `.songcage.json` layout. The reader doesn't
assume one: it walks the JSON for lyric-like and chord-like records using the
key names in `KEYMAP` at the top of `songcage_reader.py`. When a real file is
in hand:

1. Run with `--schema` to print every key path, type and a sample value.
2. Adjust `KEYMAP` (or pass `--keymap keys.json` with just the entries to
   change) so lyric records, positions and pitches are picked up.
3. If start positions can't be matched, `--by-order` lines syllables up with
   melody notes in sequence instead, and `--offset-beats` shifts positions.

If no lyric records are found in the song file, the reader falls back to lyric
events inside the MIDI file, if Song Cage wrote any.

## Tests

```
python3 -m unittest test_songcage_reader -v
```

The tests use two made-up layouts, since no real file exists yet.
