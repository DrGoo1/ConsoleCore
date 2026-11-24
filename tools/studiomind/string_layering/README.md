# StudioMind string layering (country pop)

Key-independent arrangement rules for layering stringed instruments in modern country pop, and a renderer that turns them into multi-track MIDI in any key.

Source of the technique: Nick Jongejan, "How to Layer Country Guitars" (YouTube, VmE-BQqoY4I), paraphrased from the spoken transcript. Each part in `rules.json` marks which details are from the video (`video`) and which are filled in from standard country practice (`idiom`), because the video plays the actual notes without saying them.

## How parts are written without a key
- **Chord tones** (`R`, `3`, `5`, `+12`) follow the current chord: bass, acoustic drive, power chords.
- **Chord voicing steps** (`v1`, `v2`...) pick notes from the chord voiced upward from the part's register floor: mandolin, banjo roll.
- **Scale degrees** (`[degree, octave]`) follow the key and ignore chord changes: the repeating acoustic anchor and the electric pedal note.
- **Guitar shapes**: open-position fret shapes for every diatonic chord in the three capo keys G, C and D. The renderer picks the shape key and capo so shape + capo = song key: lowest capo (0 to 7) for the main pair, a different shape near fret 7 for the high capo layer (top four strings only), and the main shape for the Nashville-tuned guitar.

## Run
```
python3 string_layer.py --key Bb --out song_Bb.mid --csv song_Bb.csv
python3 string_layer.py --key E --form "verse:I V vi IV|chorus:IV I V vi|bridge:vi IV|chorus:IV I V vi" --bpm 104
python3 -m unittest test_string_layer
```
`examples/` holds the worked example in G and Bb and a note sheet comparing them.

Not rendered yet (rule only): the intro hook, lead lines and slide, which need the song's melody and vocal timing.
