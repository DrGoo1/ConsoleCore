# Handoff to ChatGPT: tape machine models in ConsoleCore (2026-11-21)

From Claude, for ChatGPT, at David's request. Everything here is in
[ConsoleCore draft PR #3](https://github.com/DrGoo1/ConsoleCore/pull/3), branch
`agent/claude-physical-tape-machines`. Please do not merge it; David decides.

## What exists now

- `tape::PhysicalTapeMachine` (`modules/tape_core/`): a new tape engine beside the original
  `TapeTrackProcessor`, which is unchanged.
- Two machine profiles in `tape_profiles/machines/`:
  - `closed_loop_16track_2inch_1970.json` — benchmarked against the 3M M56 manual.
  - `transformerless_servo_24track_2inch_1979.json` — benchmarked against the Ampex ATR-124.
    **Provisional**: only response, noise and flutter come from a published source.
- `tools/tape_machine_bench`: `measure` and `render` commands. Builds without JUCE.
- `IntegratedAnalogPathEngine::setTrackMachine(spec, run)` selects a machine for the track
  tape stage and `latencySamples()` reports its delay (99 samples).
- CI moved from the retired `macos-14` runner to `macos-15`.
- Full details, sources and limits: `docs/TAPE_MACHINE_MODELS.md`.

**Nothing has been listened to and no hardware was measured.** The tests only show the models
sit inside figures from documents.

## What ChatGPT needs to do

### 1. Make listening renders on the Mac (most important)

```
cd <ConsoleCore checkout> && git fetch && git checkout agent/claude-physical-tape-machines
cmake -S tools/tape_machine_bench -B build-tape && cmake --build build-tape
ctest --test-dir build-tape --output-on-failure
./build-tape/tape_machine_bench measure tape_profiles/machines/closed_loop_16track_2inch_1970.json
./build-tape/tape_machine_bench render tape_profiles/machines/closed_loop_16track_2inch_1970.json in.wav out_m56_15ips.wav --ips 15 --drive 6
```

- Use David's own recordings as input (WAV, 16/24/32-bit or float). Keep audio out of the repo.
- Render each machine at 15 and 30 ips, at `--drive` 0, 6 and 12, and the 16-track once with
  `--linearizer-off`. Put the files where David can audition them and note his reactions.
- Report back in the PR: what sounds wrong, especially low-frequency distortion when driven,
  treble behaviour at 7.5 ips, and whether flutter is audible.

### 1b. Added 2026-11-22: tape formulas and bias (branch `agent/claude-tape-formulas-bias`)

After the renders above, check out `agent/claude-tape-formulas-bias`, rebuild, and add:

```
F=tape_profiles/formulas; M=tape_profiles/machines/transformerless_servo_24track_2inch_1979.json
./build-tape/tape_machine_bench render $M in.wav out_1975tape.wav  --ips 15 --drive 6 --formula $F/high_output_1975.json
./build-tape/tape_machine_bench render $M in.wav out_1998tape.wav  --ips 15 --drive 9 --formula $F/ultra_high_output_1998.json
./build-tape/tape_machine_bench render $M in.wav out_underbias.wav --ips 15 --drive 6 --formula $F/high_output_1975.json --bias -2
./build-tape/tape_machine_bench render $M in.wav out_overbias.wav  --ips 15 --drive 6 --formula $F/high_output_1975.json --bias 2
```

Ask David whether the bias control's range feels right: its curve shape is an assumption.
If you can find full data sheets for the 1960s tape (3M 202/203) or a bias-versus-output
graph for any of these tapes, report the numbers; they would replace assumptions.

### 2. Get the ATR-116/124 service manual

- Claude could not download it. The link is on
  https://www.historyofrecording.com/AMPEX_ATR-116_and_ATR-124.html — try it in a browser.
- Keep the PDF **outside the repo** (for example in this MP 1 folder).
- From its specifications section, record with page numbers: distortion and the level it is
  quoted at, operating level, equalization per speed, bias frequency, maximum output, signal
  to noise with its reference level, flutter with its weighting, and 7.5 ips figures.
- Put the numbers in the PR as a comment, or edit the profile's values and `evidence` notes.
  Every evidence note must start with `published:`, `standard:`, `derived:` or `assumed:`.

### 3. Review the PR

- Check the code against `docs/LEGAL_MODELING_BOUNDARIES.md` and `.github/copilot-instructions.md`.
- Challenge any `assumed:` value you can replace with a documented one.

## Rules that apply to both of us

- Do not decompile, disassemble or copy the Pulsar Modular P821 plug-in or any other
  commercial plug-in. Listening to it as a reference is fine; copying its curves is not.
- Do not commit manuals, audio or third-party material. Profiles carry numbers and where
  they came from, nothing more.
- Do not describe a model as matching or equalling a real machine.
- One branch per agent. Please do not push to `agent/claude-physical-tape-machines`; open
  your own branch from it and a PR, or comment.

## Planned next, and who

| Work | Owner |
| --- | --- |
| Listening renders, ATR manual, PR review | ChatGPT |
| Tape formulas and a working bias control in the engine | Claude: done, in review |
| Rebuild the console stage (oversampled, level-calibrated, frequency-dependent) | Claude |
| Plug-in parameters (machine, speed, drive, linearizer) and host latency reporting | Open: say in the PR if you want it |

## Known weaknesses found in the existing engine (for context)

- The standalone app's engine is a pass-through and the plug-ins are stubs.
- `AnalogPhysicsCore` and `TapeTrackProcessor` add +19 dB and +11.5 dB of gain on quiet
  signals at default settings, have no frequency dependence, and alias heavily when driven
  (no oversampling). `AnalogPhysicsCore` also adds a DC offset.
- Wow, flutter and crosstalk in `TapeTrackProcessor` are telemetry only; `tape_profiles/Generic_*.json`
  are not loaded by any code.
