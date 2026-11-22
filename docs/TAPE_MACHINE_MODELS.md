# Physical tape machine models

A second tape engine, `tape::PhysicalTapeMachine`, sits beside the original generic
`TapeTrackProcessor`. It is driven by a machine profile (JSON, schema `tapecore.machine.v2`)
in which every sound-shaping value carries a note saying where it came from.

**Status: documentation-derived. No hardware was measured and nobody has listened to these
models yet.** Passing the bench means the model sits inside figures taken from documents. It
does not mean the model sounds like a real machine. Names follow
`docs/LEGAL_MODELING_BOUNDARIES.md`: profiles are descriptive families, and the reference
machine is named only as the source of benchmark figures.

## Profiles

| Profile | Benchmark source | State |
| --- | --- | --- |
| `closed_loop_16track_2inch_1970` | 3M M56 maintenance manual and the 1968 AES paper on the two-inch 16-track recorder | Published response, distortion, flutter, bias and line-amp figures are used. Coating thickness, head bump, flutter spectrum, noise and the 30 ips curve are assumptions. |
| `transformerless_servo_24track_2inch_1979` | Ampex ATR-124: Studio Sound multitrack survey (January 1980) and manufacturer advertising | **Provisional.** Published response (25 Hz to 20 kHz at 15 ips, 40 Hz to 30 kHz at 30 ips, both +/-2 dB), noise and flutter are used. The service manual could not be retrieved, so distortion, levels, bias, head data and everything at 7.5 ips are assumptions. |

## Signal path

input high-pass → input transformer (if fitted) → 4x oversampling → record equalization
(NAB or AES) → short-wavelength pre-emphasis → linearizer (if fitted) and tape saturation →
short-wavelength loss → tape noise (optional) → wow, flutter and scrape (one transport,
shared by all channels) → reproduce equalization → reproduce head high-frequency limit →
downsampling → head bump → output transformer (if fitted) → output ceiling.

What follows from the physics rather than from tuning:

- The head bump is fixed in wavelength, so it doubles in frequency from 15 to 30 ips.
- Short-wavelength loss scales with speed, so treble saturates earlier at slower speeds.
- Saturation drive is solved so the third harmonic at 1 kHz equals the profile's figure at
  the profile's level. The linearizer amount is solved the same way.
- Transformer distortion rises as frequency falls and level rises.

## Measured (48 kHz, from `tape_machine_bench measure`)

Closed-loop 16-track profile:

| Check | 7.5 ips | 15 ips | 30 ips | Published limit |
| --- | --- | --- | --- | --- |
| Response, 100 Hz to top of band | +0.0 / -0.6 dB | +0.0 / -0.4 dB | none published | +/-2 dB |
| Response, 30 to 50 Hz | +0.5 / -0.2 dB | +0.8 / -0.4 dB | none published | +2 / -4 dB |
| Third harmonic at +6 dB, linearizer off | 3.00 % | 3.00 % | 3.00 % | 3 % |
| Third harmonic at +6 dB, linearizer on | 0.70 % | 0.70 % | 0.70 % | under 0.8 % |
| Flutter, 0.5 to 300 Hz | 0.036 % | 0.023 % | 0.024 % | 0.07 % / 0.04 % / none |

Transformerless servo 24-track profile: response +0.7 / -0.9 dB at 15 ips and inside +/-2 dB
at 30 ips (checked to 30 kHz at a 96 kHz sample rate); flutter 0.017 % and 0.019 % against a
published 0.03 % weighted figure, which is a different measurement from the bench's
unweighted rms.

Latency is 99 samples at every sample rate and is reported by `latencySamples()`.

## Tape formulas

A formula (`tape_profiles/formulas/*.json`, schema `tapecore.formula.v1`) replaces the
machine's tape values: coating thickness, the level of 3 % third harmonic, and the bias
alignment point. Heads, electronics and transport stay the machine's. In code:
`tape::withFormula(machineSpec, formula)`. Levels are relative to 185 nWb/m.

| Formula | Benchmark source | 3 % level | Coating | Model at operating level | Data sheet |
| --- | --- | --- | --- | --- | --- |
| `low_noise_standard_output_1960s` | 3M M56 manual's figure on its recommended tape (3M 202/203) | +6 dB (derived) | 12.7 um (assumed) | 0.80 % | none |
| `high_output_1975` | Ampex/Quantegy 456 data sheet | +12.3 dB | 12.7 um | 0.19 % | 0.1 % |
| `ultra_high_output_1998` | Quantegy GP9 data sheet | +17.5 dB | 17.7 um | 0.059 % | 0.06 % |

The last two columns are an independent check: the data-sheet distortion at operating level
was not used to build the model. The 1998 tape matches; the 1975 tape reads about twice the
sheet, so the simple saturation curve is slightly too soft low down for that tape.

To run hotter on a high-output tape, as studios did by aligning at +3, +6 or +9, use the
drive setting: it raises level onto tape and lowers it after, so noise falls by the same amount.

## Bias

`RunSettings::biasOffsetDb` moves bias from -3 to +3 dB around the aligned setting. The
machine's equalization stays as aligned, so:

- Under-bias lifts the top end and raises distortion.
- Over-bias dulls the top end.
- The effect follows wavelength, so it sits an octave higher at 30 ips than at 15 ips.

One point is from documents: taking bias back to the peak restores the overbias figure
(3 dB at 10 kHz at 15 ips for the 1975 tape; the model gives +2.99 dB). **The shape of the
curve either side, the 3 dB bias range from peak to alignment, and the size of the
distortion change are assumptions.** Bias is still not simulated as a signal.

## Limits

- Bias is not simulated as a signal. Saturation is a memoryless anhysteretic curve, so there
  is no hysteresis memory, no bias-dependent treble behaviour and no modulation noise.
- The published distortion figures are tied to "+6 dB over operating level". The manual
  states them at +10 dBm; the mapping to tape level is an assumption recorded in the profile.
- Low-frequency distortion at high level (30 Hz) is large in this model because the record
  bass boost drives the tape harder there. It is plausible but unverified: listen for it.
- No crosstalk between tracks, no sync-head playback, no dropouts, no azimuth error.
- Connected to `IntegratedAnalogPathEngine` as the track tape stage: call
  `setTrackMachine(spec, run)` to select a machine and report `latencySamples()` to the host.
  Without that call the generic processor runs as before. The print tape stage, the console
  stage and the plug-ins are not connected yet, and there are no user parameters.
- The flutter spectrum (which frequencies) is assumed; only the total is published.

## Build and run without JUCE

```
cmake -S tools/tape_machine_bench -B build-tape && cmake --build build-tape
ctest --test-dir build-tape --output-on-failure
./build-tape/tape_machine_bench measure tape_profiles/machines/closed_loop_16track_2inch_1970.json
./build-tape/tape_machine_bench render  tape_profiles/machines/closed_loop_16track_2inch_1970.json in.wav out.wav --ips 15 --drive 6
```

`render` options: `--ips`, `--drive` (dB), `--formula <tape.json>`, `--bias` (dB), `--noise`,
`--no-flutter`, `--linearizer-off`. `measure` also accepts `--formula`.

## Next

1. Listen to renders and compare against any tape recordings available.
2. Obtain the ATR-124 service manual (the public copy would not download) and replace the remaining assumed values.
3. Expose machine, speed, drive and linearizer as plug-in parameters, and report latency from the plug-in.
4. Add hysteresis with bias if listening shows the simple curve is not enough.
5. Rebuild the console stage to the same standard (oversampled, level-calibrated, frequency-dependent).
