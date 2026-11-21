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
| `transformerless_servo_24track_2inch_1979` | Ampex ATR-124 (manufacturer advertising only) | **Provisional.** The service manual has not been read, so every performance value is an assumption and there are no published limits to test against. |

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

Latency is 99 samples at every sample rate and is reported by `latencySamples()`.

## Limits

- Bias is not simulated as a signal. Saturation is a memoryless anhysteretic curve, so there
  is no hysteresis memory, no bias-dependent treble behaviour and no modulation noise.
- The published distortion figures are tied to "+6 dB over operating level". The manual
  states them at +10 dBm; the mapping to tape level is an assumption recorded in the profile.
- Low-frequency distortion at high level (30 Hz) is large in this model because the record
  bass boost drives the tape harder there. It is plausible but unverified: listen for it.
- No crosstalk between tracks, no sync-head playback, no dropouts, no azimuth error.
- Not yet connected to `IntegratedAnalogPathEngine` or to any plug-in parameter.
- The flutter spectrum (which frequencies) is assumed; only the total is published.

## Build and run without JUCE

```
cmake -S tools/tape_machine_bench -B build-tape && cmake --build build-tape
ctest --test-dir build-tape --output-on-failure
./build-tape/tape_machine_bench measure tape_profiles/machines/closed_loop_16track_2inch_1970.json
./build-tape/tape_machine_bench render  tape_profiles/machines/closed_loop_16track_2inch_1970.json in.wav out.wav --ips 15 --drive 6
```

`render` options: `--ips`, `--drive` (dB), `--noise`, `--no-flutter`, `--linearizer-off`.

## Next

1. Listen to renders and compare against any tape recordings available.
2. Read the ATR-124 service manual and replace the assumed values.
3. Connect the engine to `IntegratedAnalogPathEngine` behind a machine selector.
4. Add hysteresis with bias if listening shows the simple curve is not enough.
