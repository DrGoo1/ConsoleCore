# StudioMind knowledge pack: country pop string layering

**What this is:** arrangement rules for layering guitars, mandolin and banjo in modern country pop. StudioMind uses them when it generates MIDI parts for a song in any key. They are paraphrased technique from Nick Jongejan's video "How to Layer Country Guitars" (https://youtu.be/VmE-BQqoY4I). No notes from his song are included.

**Schema:** `studiomind.string_layering/0.1` (the JSON block at the end). Source of truth: ConsoleCore repo, branch `claude/project-thread-j71247`, file `tools/studiomind/string_layering/rules.json`. A reference renderer, `string_layer.py`, sits beside it.

## Instructions for the model ingesting this

1. Treat the JSON block as the authoritative rule set. The prose is there to explain it.
2. Never store fixed note names for a part. Resolve every note from the song's key and the current chord at render time, using the conventions below.
3. Each field carries a `provenance` of `video` (stated in the video) or `idiom` (standard modern country practice, filled in because the video plays the notes without saying them). Idiom values are defaults, so they may be overridden by the song or by the user. Video values are the core technique.
4. Apply parts per section, using each part's `sections` map. A part with no entry for a section stays silent there.
5. Lead, slide and the intro hook are in `rule_only_parts`. They need the song's melody and vocal timing, so only place them when those are known.

## Key-independent conventions

- **Chord progression:** Roman numerals per bar (I ii iii IV V vi vii), major key.
- **Chord tones:** `R`, `3`, `5` = root, third and fifth of the current chord (the third is major or minor by chord quality), placed at the lowest pitch at or above the part's `register` floor (MIDI note numbers). `+12` = up an octave. A list of tokens in one step = played together.
- **Voicing steps:** `v1`, `v2`, ... = the 1st, 2nd and so on notes of the current chord, voiced upward from the register floor (v1 = lowest chord tone at or above the floor). A list cycles one token per hit.
- **Scale degrees:** `[degree, octave]` = a degree of the major key, which ignores chord changes (used for the anchor and pedal parts).
- **Patterns:** 16 steps per 4/4 bar. Strummed parts: `D` down (all strings low to high), `U` up (top four strings high to low), `d` ghost, `.` rest. Note parts: `o` hit, `x` short muted hit, `.` rest. `gate` = note length in 16ths. `hold` = ring to the bar's end.
- **Guitar shapes:** `shape_keys` gives open-position fret shapes (low E to high E, -1 = muted) for every diatonic chord in the keys G, C and D. Sounding pitch = tuning + fret + capo. Shape key + capo = song key.
- **Capo choice:** the main pair (and the Nashville-tuned guitar) uses the shape key needing the lowest capo, 0 to 7, preferring G, then C, then D. The high layer uses a different shape key with the capo nearest fret 7 (between 4 and 9), and only its top four strings.
- **Nashville tuning:** standard tuning with the lower four strings an octave up (MIDI 52 57 62 67 59 64). Unison notes sound once.
- **Pan:** -100 = hard left, 100 = hard right.
- **Timing:** drive parts (bass and acoustic 16ths) humanize about 2 ms; other layers 5 to 8 ms. Hits line up closely but not perfectly.

### Capo by key

| Key | Main pair and Nashville | High capo layer |
| --- | --- | --- |
| C | C shape, capo 0 | G shape, capo 5 |
| Db | C shape, capo 1 | G shape, capo 6 |
| D | D shape, capo 0 | G shape, capo 7 |
| Eb | D shape, capo 1 | G shape, capo 8 |
| E | D shape, capo 2 | G shape, capo 9 |
| F | D shape, capo 3 | C shape, capo 5 |
| F# | D shape, capo 4 | C shape, capo 6 |
| G | G shape, capo 0 | C shape, capo 7 |
| Ab | G shape, capo 1 | D shape, capo 6 |
| A | G shape, capo 2 | D shape, capo 7 |
| Bb | G shape, capo 3 | D shape, capo 8 |
| B | G shape, capo 4 | D shape, capo 9 |

## The rules in plain words

1. **Section contrast (video):** verses are acoustic-only and narrower; the chorus adds electric power chords, more acoustic layers and full stereo width.
2. **16th-note engine (video):** bass and a muted acoustic play straight 16ths together, tight and with flat velocity.
3. **Anchor (video: never changes; idiom: notes):** one acoustic repeats the same notes and pattern over every chord in the verse, using scale degrees 1, 5, 9 and 5. It sits about 64% to one side.
4. **Shine (video):** extra chorus layers restate the same chords higher: a capo voicing, a Nashville-tuned guitar, and a mandolin holding just the I chord for the whole chorus.
5. **Rhythmic relief (video):** against all the 16ths, verse 2 adds an electric pedal note in 8ths (idiom: the key's tonic).
6. **Banjo (video: rolling; idiom: roll):** a 16th-note forward roll on chord tones through the chorus.
7. **Power chords (video):** chorus only, doubled hard left and right; root, fifth and octave (idiom: accented 8ths with muted chugs between).
8. **Gaps, not stacks (video):** leads go between vocal phrases, and slide between lead phrases. Two melodic parts in the same register never play at once.
9. **Width as dynamics (video):** verse guitars sit about 64% to a side; chorus doubles go hard left and right.
10. **Bridge (video):** one doubled guitar keeps its pattern, and the other guitars hold whole-note chords.

## Check data: chorus over the I chord, notes low to high

| Part | Key of G | Key of Bb |
| --- | --- | --- |
| Acoustic pair | G2 B2 D3 G3 B3 G4 | Bb2 D3 F3 Bb3 D4 Bb4 |
| Capo acoustic | B3 D4 G4 B4 | Bb3 F4 Bb4 D5 |
| Nashville acoustic | G3 B3 D4 G4 | Bb3 D4 F4 Bb4 |
| Mandolin (one chord) | G4 B4 D5 G5 | Bb4 D5 F5 Bb5 |
| Banjo roll | G3 B3 G4 | Bb3 D4 Bb4 |
| Power chords | G2 D3 G3 | Bb2 F3 Bb3 |
| Verse anchor | G3 D4 A4 D5 | Bb3 F4 C5 F5 |

Not covered by the video: fiddle, steel guitar and dobro, the exact strum and picking patterns, and tones and effects. The only tones and effects mentioned are grit on the chorus bass and heavy reverb on the post-chorus electric.

## Rules (JSON)

```json
{
  "schema": "studiomind.string_layering/0.1",
  "source": {
    "title": "How to Layer Country Guitars",
    "channel": "Nick Jongejan",
    "url": "https://youtu.be/VmE-BQqoY4I",
    "note": "Technique only, paraphrased from the spoken transcript. 'video' = said in the video; 'idiom' = standard modern-country practice filled in because the video plays it without saying it."
  },
  "grid": {
    "steps_per_bar": 16,
    "beats_per_bar": 4
  },
  "tunings": {
    "guitar_standard": [
      40,
      45,
      50,
      55,
      59,
      64
    ],
    "guitar_nashville": [
      52,
      57,
      62,
      67,
      59,
      64
    ],
    "banjo6": [
      40,
      45,
      50,
      55,
      59,
      64
    ]
  },
  "shape_keys": {
    "_doc": "Open-position fret shapes (low E to high E, -1 = muted) for each diatonic chord, in the three keys guitarists capo from. Key of the song = shape key + capo.",
    "G": {
      "offset": 7,
      "I": [
        3,
        2,
        0,
        0,
        0,
        3
      ],
      "ii": [
        -1,
        0,
        2,
        2,
        1,
        0
      ],
      "iii": [
        -1,
        2,
        4,
        4,
        3,
        2
      ],
      "IV": [
        -1,
        3,
        2,
        0,
        1,
        0
      ],
      "V": [
        -1,
        -1,
        0,
        2,
        3,
        2
      ],
      "vi": [
        0,
        2,
        2,
        0,
        0,
        0
      ]
    },
    "C": {
      "offset": 0,
      "I": [
        -1,
        3,
        2,
        0,
        1,
        0
      ],
      "ii": [
        -1,
        -1,
        0,
        2,
        3,
        1
      ],
      "iii": [
        0,
        2,
        2,
        0,
        0,
        0
      ],
      "IV": [
        -1,
        -1,
        3,
        2,
        1,
        1
      ],
      "V": [
        3,
        2,
        0,
        0,
        0,
        3
      ],
      "vi": [
        -1,
        0,
        2,
        2,
        1,
        0
      ]
    },
    "D": {
      "offset": 2,
      "I": [
        -1,
        -1,
        0,
        2,
        3,
        2
      ],
      "ii": [
        0,
        2,
        2,
        0,
        0,
        0
      ],
      "iii": [
        -1,
        -1,
        4,
        2,
        2,
        2
      ],
      "IV": [
        3,
        2,
        0,
        0,
        0,
        3
      ],
      "V": [
        -1,
        0,
        2,
        2,
        2,
        0
      ],
      "vi": [
        -1,
        2,
        4,
        4,
        3,
        2
      ]
    }
  },
  "capo_rules": {
    "main": {
      "provenance": "idiom",
      "rule": "Pick the shape key (G, C or D, in that order of preference) that needs the lowest capo, 0 to 7.",
      "max_capo": 7,
      "prefer": [
        "G",
        "C",
        "D"
      ]
    },
    "high": {
      "provenance": "video+idiom",
      "rule": "Video: same chords higher up the neck with a capo. Idiom: use a different shape key with the capo nearest fret 7, between 4 and 9.",
      "min_capo": 4,
      "max_capo": 9,
      "target": 7
    },
    "nashville": {
      "provenance": "video+idiom",
      "rule": "Video: the chorus part again in Nashville tuning. Idiom: same shape and capo as the main pair; the tuning lifts the lower four strings an octave."
    }
  },
  "strum": {
    "down": "all sounding strings low to high",
    "up": "top four strings high to low",
    "ghost": "top three strings, half velocity",
    "stagger_ms": 9
  },
  "parts": [
    {
      "id": "bass",
      "name": "Bass",
      "program": 33,
      "provenance": {
        "rhythm_verse": "video",
        "notes": "idiom",
        "chorus": "idiom"
      },
      "mode": "tones",
      "register": [
        33,
        52
      ],
      "tones": [
        "R"
      ],
      "sections": {
        "verse": {
          "pattern": "oooooooooooooooo",
          "gate": 0.9,
          "vel": 92,
          "pan": 0
        },
        "chorus": {
          "pattern": "o.o.o.o.o.o.o.o.",
          "gate": 0.95,
          "vel": 96,
          "pan": 0,
          "approach": true
        },
        "bridge": {
          "pattern": "o...............",
          "gate": 15.5,
          "vel": 90,
          "pan": 0
        }
      },
      "humanize_ms": 2,
      "vel_jitter": 2
    },
    {
      "id": "ac_drive",
      "name": "Acoustic drive (16ths)",
      "program": 25,
      "provenance": {
        "rhythm": "video",
        "notes": "idiom"
      },
      "mode": "tones",
      "register": [
        40,
        57
      ],
      "tones": [
        [
          "R",
          "5"
        ]
      ],
      "sections": {
        "verse": {
          "pattern": "oooooooooooooooo",
          "gate": 0.55,
          "vel": 78,
          "pan": -30
        }
      },
      "humanize_ms": 2,
      "vel_jitter": 2
    },
    {
      "id": "ac_anchor",
      "name": "Acoustic anchor (repeats)",
      "program": 25,
      "provenance": {
        "repeat_over_changes": "video",
        "pan_64": "video",
        "notes_and_pattern": "idiom"
      },
      "mode": "degree_strum",
      "register": [
        47,
        80
      ],
      "degrees": [
        [
          1,
          0
        ],
        [
          5,
          0
        ],
        [
          2,
          1
        ],
        [
          5,
          1
        ]
      ],
      "sections": {
        "verse": {
          "pattern": "D...D.U...U.D.U.",
          "vel": 84,
          "pan": 64
        }
      },
      "humanize_ms": 8,
      "vel_jitter": 5
    },
    {
      "id": "mando_verse",
      "name": "Mandolin (verse 16ths)",
      "program": 25,
      "provenance": {
        "rhythm": "video",
        "notes": "idiom"
      },
      "mode": "tones",
      "register": [
        67,
        88
      ],
      "tones": [
        "v2",
        "v3",
        "v4",
        "v3"
      ],
      "sections": {
        "verse": {
          "pattern": "oooooooooooooooo",
          "gate": 0.8,
          "vel": 72,
          "pan": -64
        }
      },
      "humanize_ms": 6,
      "vel_jitter": 6
    },
    {
      "id": "el_pedal",
      "name": "Electric pedal (8ths)",
      "program": 27,
      "provenance": {
        "eighths_against_16ths": "video",
        "which_note": "idiom"
      },
      "mode": "degree_tones",
      "register": [
        59,
        72
      ],
      "degrees": [
        [
          1,
          0
        ]
      ],
      "sections": {
        "verse": {
          "pattern": "o.o.o.o.o.o.o.o.",
          "gate": 1.8,
          "vel": 80,
          "pan": -40,
          "only_instances": [
            2
          ]
        }
      },
      "humanize_ms": 6,
      "vel_jitter": 4
    },
    {
      "id": "ac_pair_L",
      "name": "Acoustic pair L (open)",
      "program": 25,
      "provenance": {
        "open_voicing_double_hard_LR": "video",
        "pattern": "idiom",
        "keeps_playing_in_bridge": "video"
      },
      "mode": "shape_strum",
      "shape": "main",
      "tuning": "guitar_standard",
      "sections": {
        "chorus": {
          "pattern": "D...D.DU..UD.DU.",
          "vel": 92,
          "pan": -100
        },
        "bridge": {
          "pattern": "D...D.DU..UD.DU.",
          "vel": 86,
          "pan": -100
        }
      },
      "humanize_ms": 8,
      "vel_jitter": 5
    },
    {
      "id": "ac_pair_R",
      "name": "Acoustic pair R (open)",
      "program": 25,
      "copy_of": "ac_pair_L",
      "sections": {
        "chorus": {
          "pattern": "D...D.DU..UD.DU.",
          "vel": 92,
          "pan": 100
        },
        "bridge": {
          "pattern": "D...D.DU..UD.DU.",
          "vel": 86,
          "pan": 100
        }
      },
      "humanize_ms": 8,
      "vel_jitter": 5
    },
    {
      "id": "ac_capo",
      "name": "Acoustic capo (high voicing)",
      "program": 25,
      "provenance": {
        "same_chords_higher_with_capo": "video",
        "shape_choice": "idiom",
        "whole_notes_in_bridge": "video",
        "top_four_strings": "idiom"
      },
      "mode": "shape_strum",
      "shape": "high",
      "tuning": "guitar_standard",
      "sections": {
        "chorus": {
          "pattern": "D...D.DU..UD.DU.",
          "vel": 84,
          "pan": -70
        },
        "bridge": {
          "pattern": "D...............",
          "vel": 80,
          "pan": -70,
          "hold": true
        }
      },
      "humanize_ms": 8,
      "vel_jitter": 5,
      "strings": "top4"
    },
    {
      "id": "ac_nashville",
      "name": "Acoustic Nashville tuning",
      "program": 25,
      "provenance": {
        "part": "video",
        "tuning_detail": "idiom",
        "whole_notes_in_bridge": "video"
      },
      "mode": "shape_strum",
      "shape": "nashville",
      "tuning": "guitar_nashville",
      "sections": {
        "chorus": {
          "pattern": "D...D.DU..UD.DU.",
          "vel": 80,
          "pan": 70
        },
        "bridge": {
          "pattern": "D...............",
          "vel": 76,
          "pan": 70,
          "hold": true
        }
      },
      "humanize_ms": 8,
      "vel_jitter": 5
    },
    {
      "id": "mando_chorus",
      "name": "Mandolin (one chord)",
      "program": 25,
      "provenance": {
        "one_chord_all_chorus": "video",
        "which_chord_and_rhythm": "idiom"
      },
      "mode": "static_strum",
      "chord": "I",
      "register": [
        67,
        88
      ],
      "tones": [
        "v1",
        "v2",
        "v3",
        "v4"
      ],
      "sections": {
        "chorus": {
          "pattern": "D.U.D.U.D.U.D.U.",
          "vel": 74,
          "pan": -50
        }
      },
      "humanize_ms": 6,
      "vel_jitter": 6
    },
    {
      "id": "banjo",
      "name": "Banjo roll (6-string)",
      "program": 105,
      "provenance": {
        "rolling_banjo_chorus": "video",
        "roll_shape": "idiom"
      },
      "mode": "tones",
      "register": [
        55,
        79
      ],
      "tones": [
        "v1",
        "v2",
        "v4",
        "v1",
        "v2",
        "v4",
        "v1",
        "v4"
      ],
      "sections": {
        "chorus": {
          "pattern": "oooooooooooooooo",
          "gate": 1.6,
          "vel": 78,
          "pan": 50
        }
      },
      "humanize_ms": 6,
      "vel_jitter": 8
    },
    {
      "id": "el_power_L",
      "name": "Electric power chords L",
      "program": 29,
      "provenance": {
        "chorus_only_power_chords": "video",
        "chug_pattern": "idiom",
        "whole_notes_in_bridge": "video"
      },
      "mode": "tones",
      "register": [
        40,
        64
      ],
      "tones": [
        [
          "R",
          "5",
          "R+12"
        ]
      ],
      "sections": {
        "chorus": {
          "pattern": "o.x.o.x.o.x.o.x.",
          "gate": 1.9,
          "x_gate": 0.6,
          "vel": 100,
          "pan": -100
        },
        "bridge": {
          "pattern": "o...............",
          "gate": 15.5,
          "vel": 88,
          "pan": -100
        }
      },
      "humanize_ms": 5,
      "vel_jitter": 4
    },
    {
      "id": "el_power_R",
      "name": "Electric power chords R",
      "program": 29,
      "copy_of": "el_power_L",
      "sections": {
        "chorus": {
          "pattern": "o.x.o.x.o.x.o.x.",
          "gate": 1.9,
          "x_gate": 0.6,
          "vel": 100,
          "pan": 100
        },
        "bridge": {
          "pattern": "o...............",
          "gate": 15.5,
          "vel": 88,
          "pan": 100
        }
      },
      "humanize_ms": 5,
      "vel_jitter": 4
    }
  ],
  "rule_only_parts": [
    {
      "id": "hook",
      "provenance": "video",
      "rule": "Intro signature lick on acoustic with ringing open strings; repeat it on electric with heavy reverb after the chorus. Notes not in the transcript."
    },
    {
      "id": "lead",
      "provenance": "video",
      "rule": "Lead lines from verse 2 on, placed only in gaps between vocal phrases. Idiom: major pentatonic of the key. Needs the vocal's timing to place."
    },
    {
      "id": "slide",
      "provenance": "video",
      "rule": "Slide guitar in the last chorus and outro, answering in the gaps between lead phrases, never over them. Idiom: slide into 3, 5 or 6 from a whole step below."
    }
  ],
  "global_rules": [
    {
      "id": "section_contrast",
      "provenance": "video",
      "rule": "Verses acoustic-only and narrower; chorus adds electric power chords, more acoustic layers, full width."
    },
    {
      "id": "align_transients",
      "provenance": "video",
      "rule": "Hits across layers line up closely but not perfectly. Drive parts jitter about 2 ms, other layers 5 to 8 ms."
    },
    {
      "id": "width_as_dynamics",
      "provenance": "video",
      "rule": "Verse guitars about 64% to a side; chorus doubles hard left and right."
    },
    {
      "id": "bridge_open",
      "provenance": "video",
      "rule": "One doubled guitar keeps its pattern; the other guitars hold whole-note chords."
    },
    {
      "id": "same_register_gaps",
      "provenance": "video",
      "rule": "Parts in the same frequency area never play melody at the same time."
    }
  ],
  "token_doc": {
    "R/3/5": "chord tone of the current chord, lowest at or above the register floor; +12 = an octave up",
    "vN": "Nth note of the chord voiced upward from the register floor (v1 = lowest chord tone at or above the floor)",
    "degrees": "[scale degree of the key, octave] - ignores chord changes",
    "patterns": "16 steps per bar. Strums: D down, U up, d ghost. Notes: o hit, x short muted hit. '.' rest."
  }
}
```
