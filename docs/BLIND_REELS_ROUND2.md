# Blind reels, round 2

Round 1 drove both machines far past what their tape could take (+6 dB into tape rated about
+6 dB, and the ATR's own tape was set 3 dB hotter than the 1975 data sheet), so every tape version
sounded compressed and dull, and the two machines sounded alike. The fix in this branch puts both
machines on published tape figures. Round 2 checks that by ear, and adds speed and tape formula.

## What is in it

Two sources, each cut to the same 15 s:

- **Angelmaker full mix**
- **Softly Bleed drum overheads**

Five reels per source, 32 clips, about 8 minutes in one pass:

| Reel | Question | Versions (shuffled to A, B, C, D) |
|---|---|---|
| R1 | Do the machines differ at normal level? | dry, ATR, M56: 1975 tape, 15 ips, drive 0 |
| R2 | ...when pushed a little? | dry, ATR, M56: 1975 tape, 15 ips, drive +3 dB |
| R3 | Same as R1 at 30 ips | dry, ATR, M56: 1975 tape, 30 ips, drive 0 |
| R4 | Same as R2 at 30 ips | dry, ATR, M56: 1975 tape, 30 ips, drive +3 dB |
| R5 | Does the tape formula matter? | dry, ATR at 15 ips, drive +3 dB on 1960s standard, 1975 high output and 1998 ultra high output tape |

"1975 tape" is the high-output formula (456 type), "1998" the ultra-high-output (GP9 type),
"1960s" the standard-output (202/203 type). ATR is the transformerless 24-track, M56 the
transformer-coupled 16-track. Flutter is on and hiss is off in every version.

What to expect if the fix is right: at drive 0 the tape versions should sound close to dry, with
no obvious loss of top end. M56 should have a little more low-end weight (bigger head bump) and a
slightly softer top than the ATR. At +3 dB the 1960s tape should be clearly the most saturated and
the 1998 tape the cleanest. 30 ips should move the low-end bump up and tighten the bass.

## Rules for listening

- Every file is matched to -24 LUFS, dry included, so louder never wins.
- Listen to each reel before opening the key. Write down for each letter: which is dry, which
  you prefer, and anything that sounds compressed or dull.
- The key is in `KEY_open_after_listening.txt`, next to the reels.

## Rendering on the Mac

The sources must be at their normal mix level (0 VU = -18 dBFS), not normalised, because the drive
settings are relative to that. Use the same files as round 1. Put them here:

```
/Volumes/MP 1/Claude_Files/ConsoleCore_blind_round2/sources/angelmaker_mix.wav
/Volumes/MP 1/Claude_Files/ConsoleCore_blind_round2/sources/softly_bleed_overheads.wav
```

Then, from the ConsoleCore checkout (on branch `claude/project-thread-5k0qaq`, after
`tools/reference_jig/setup_mac.sh` has built the bench once), with the start times in seconds of
the excerpts (here 60 and 30):

```
cd "/Volumes/MP 1/Claude_Files/ConsoleCore" && R="/Volumes/MP 1/Claude_Files/ConsoleCore_blind_round2" && "/Volumes/MP 1/Claude_Files/ConsoleCore_reference_jig/.venv/bin/python" tools/reference_jig/round2.py --mix "$R/sources/angelmaker_mix.wav:60" --overheads "$R/sources/softly_bleed_overheads.wav:30" --out "$R/reels"
```

The reels land in `ConsoleCore_blind_round2/reels/` as `mix_R1_A.wav` ... `overheads_R5_D.wav`
(48 kHz or the source rate, 24-bit, stereo), with the key beside them. `--seed N` gives a different
shuffle. The script only needs numpy.

Each tape version is one call to the bench, for example R2's ATR:

```
build-tape/tape_machine_bench render tape_profiles/machines/transformerless_servo_24track_2inch_1979.json mix_dry.wav mix_R2_ATR.wav --ips 15 --drive 3 --formula tape_profiles/formulas/high_output_1975.json --float
```

and the M56 the same with `tape_profiles/machines/closed_loop_16track_2inch_1970.json`; R5 swaps the
formula file for `low_noise_standard_output_1960s.json` or `ultra_high_output_1998.json`.
