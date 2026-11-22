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

## Profiles

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

A second profile, `american_discrete_opamp_line_1970.json`, is **provisional**. It is
benchmarked against a line amplifier built on the API 2520 discrete op-amp, using only that
op-amp's original data sheet: distortion 0.2 % at rated output (the model gives 0.18 % at
+28 dBu) and the rated output itself. Its low-level distortion, harmonic balance (mostly
third), bandwidth, noise and transformer behaviour are all assumptions.

## Measured (class-A profile, one stage, 48 kHz)

| Check | Model | Published |
| --- | --- | --- |
| Response, 20 Hz to 20 kHz | +0.02 / -0.19 dB | +/-0.5 dB |
| Response at 40 kHz (96 kHz sample rate) | -3.01 dB | -3 dB |
| Distortion at +20 dBu: 50 Hz / 1 kHz / 10 kHz | 0.060 / 0.050 / 0.048 % | at most 0.07 % |
| Output with +26 dBu in | +25.9 dBu at 1.0 % | above +26 dBu |
| Noise | -83.1 dBu | -83 dBu |
| Distortion at nominal level (+4 dBu) | 0.009 % | none |
| Distortion at 20 Hz, +20 dBu | 0.50 % | none |

Latency is 34 samples for one stage at 48 kHz. It grows slightly with stage count and
sample rate (35 for three stages; 35 at 96 kHz), so read `latencySamples()` after `prepare()`.

## Mix bus

`console::PhysicalConsoleBus` gives each stereo stem its own channel stage, sums the stems,
and passes the sum through a shared bus stage. Because the bus stage works on the sum:

- Stems intermodulate. Two stems at 1 kHz and 1.3 kHz, +16 dBu each, produce a 300 Hz
  difference tone at -48 dBu on the bus and -114 dBu when processed separately.
- A loud stem takes headroom from a quiet one (2.4 dB in an extreme test).
- The same material spread over more stems reaches the bus stage hotter and distorts more.

Channel and bus stages may use different profiles. Latency is 35 samples at 48 kHz for the
class-A profile whatever the stem count, up to 64 stems. The summing itself is ideal: no
crosstalk, no pan-law or resistor-network behaviour, no shared power-supply effects.

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
- `stages` on a single stage runs identical stages in series on each channel. For
  interaction between stems use the mix bus.
- At a 44.1 kHz sample rate the oversampling filters are 0.9 dB down at 19.8 kHz, so the
  published response is met only up to about 18.5 kHz there.
- `prepare()` takes roughly a third of a second because it calibrates. Keep it off the audio thread.

## Use

```
./build-tape/tape_machine_bench measure-console console_profiles/stages/british_class_a_discrete_line_1970.json
./build-tape/tape_machine_bench render <tape.json> in.wav out.wav --no-tape --console console_profiles/stages/british_class_a_discrete_line_1970.json --console-drive 10 --console-stages 2
./build-tape/tape_machine_bench mix <channel.json> <bus.json> out.wav stem1.wav stem2.wav ... --bus-drive 6
```

`mix` sums stem files through the bus.

Leave out `--no-tape` to run tape and then console. In code:
`IntegratedAnalogPathEngine::setConsoleStage(spec, run)`; latencies of tape and console add.

## Next

1. Listen, especially to drive and stage count.
2. Connect the mix bus to the stem runtime (`modules/runtime`) so the app can use it.
3. Firm up the American op-amp profile from a console or module manual; add a VCA large-format profile.
4. Plug-in parameters and host latency reporting.
