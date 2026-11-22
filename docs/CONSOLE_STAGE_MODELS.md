# Physical console stage model

`console::PhysicalConsoleStage` (`modules/console_physical/`) models one console amplifier
stage: transformer-coupled input, an amplifier with a calibrated transfer curve,
transformer-coupled output and a bandwidth limit, run at four times the sample rate. It is
driven by a profile (JSON, schema `consolecore.stage.v1`) in which every sound-shaping value
carries a note saying where it came from.

**Status: documentation-derived. No hardware was measured, no circuit was simulated, and
nobody has listened to this model yet.** Passing the bench means the model sits inside
figures taken from a manual. Names follow `docs/LEGAL_MODELING_BOUNDARIES.md`.

The existing `AnalogPhysicsCore`, `TopologyAwareMixEngine` and `ProductionDspEngine` are unchanged.

## Profile

`console_profiles/stages/british_class_a_discrete_line_1970.json`, benchmarked against the
line path of the Neve 1073/1084 channel amplifier as specified in its manufacturer's user manual.

| Value | Source |
| --- | --- |
| Distortion at most 0.07 %, 50 Hz to 10 kHz, at +20 dBu | published |
| Maximum output above +26 dBu | published; the 1 % criterion for it is assumed |
| Response +/-0.5 dB, 20 Hz to 20 kHz; -3 dB at 40 kHz | published; second-order roll-off assumed |
| Noise -83 dBu, 22 Hz to 22 kHz | published |
| Transformer-coupled input and output | published |
| Mostly second harmonic below the knee | **assumed** |
| Hard ceiling 2 dB above rated output | **assumed** |
| Transformer distortion below 50 Hz | **not published**; follows from the flux law |
| -18 dBFS equals +4 dBu | assumed host convention |

## Measured (one stage, 48 kHz)

| Check | Model | Published |
| --- | --- | --- |
| Response, 20 Hz to 20 kHz | +0.02 / -0.19 dB | +/-0.5 dB |
| Response at 40 kHz (96 kHz sample rate) | -3.01 dB | -3 dB |
| Distortion at +20 dBu: 50 Hz / 1 kHz / 10 kHz | 0.060 / 0.050 / 0.048 % | at most 0.07 % |
| Output with +26 dBu in | +25.9 dBu at 1.0 % | above +26 dBu |
| Noise | -83.1 dBu | -83 dBu |
| Distortion at nominal level (+4 dBu) | 0.009 % | none |
| Distortion at 20 Hz, +20 dBu | 0.50 % | none |

Latency is 34 samples (`PhysicalConsoleStage::latencySamples()`).

## What to expect when listening

At normal digital levels this stage is close to transparent, as the real specifications say
it should be: 0.009 % distortion at nominal level. Its character appears when it is driven
(`driveDb`, level-compensated), with several stages in series (`stages`, 1 to 4), and on
loud low-frequency material, where the transformers add third harmonic.

## Limits

- The transfer curve is a fitted shape, not a circuit. Only total distortion at one level is
  published; the split between second and third harmonic is an assumption.
- No frequency-dependent amplifier distortion, slew limiting, power-supply sag, crosstalk or
  channel-to-channel variation. No equalizer and no microphone amplifier.
- `stages` runs identical stages in series on each channel. It is not a summing bus: there
  is no interaction between stems.
- At a 44.1 kHz sample rate the oversampling filters are 0.9 dB down at 19.8 kHz, so the
  published response is met only up to about 18.5 kHz there.
- `prepare()` takes roughly a third of a second because it calibrates. Keep it off the audio thread.

## Use

```
./build-tape/tape_machine_bench measure-console console_profiles/stages/british_class_a_discrete_line_1970.json
./build-tape/tape_machine_bench render <tape.json> in.wav out.wav --no-tape --console console_profiles/stages/british_class_a_discrete_line_1970.json --console-drive 10 --console-stages 2
```

Leave out `--no-tape` to run tape and then console. In code:
`IntegratedAnalogPathEngine::setConsoleStage(spec, run)`; latencies of tape and console add.

## Next

1. Listen, especially to drive and stage count.
2. A real summing bus: per-stem channel stages feeding a shared mix amplifier.
3. More profiles (American discrete op-amp, VCA large-format) from their published specifications.
4. Plug-in parameters and host latency reporting.
