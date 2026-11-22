# Stem mix runtime

`PhysicalStemMixRuntime` (`modules/runtime/`) is the session signal path built from the
documentation-derived models:

```
stereo stems -> one multitrack tape machine (optional, per stem) -> console bus -> stereo
```

**Status: built from the tape and console models, which are documentation-derived and not
validated against hardware. Nobody has listened to this path yet.** See
`docs/TAPE_MACHINE_MODELS.md` and `docs/CONSOLE_STAGE_MODELS.md` for sources and limits.

## Behaviour

- One machine carries every stem, two tracks per stereo stem, so all tracks share the same
  wow and flutter.
- The machine's track count is a real limit: eight stereo stems on the 16-track profile,
  twelve on the 24-track. `prepare()` refuses more and `lastError()` says why.
- Any stem can skip the tape (`stemUsesTape`). It is delayed by the machine's latency so it
  stays aligned, to within one sample, with stems that went to tape.
- Latency is tape plus bus: 134 samples at 48 kHz with the 16-track and class-A profiles
  (measured 133 to 134), or 35 with the console alone. Read `latencySamples()` after `prepare()`.
- Meters: per-stem input peak (`stemStates()`), output peak and active stem count (`telemetry()`).
- Inputs are never modified. Any block size works, including larger than prepared.

## Not done

- Not connected to the standalone app's audio callback, to `IntegratedAnalogPathEngine`
  (which is stereo-in, stereo-out) or to any plug-in.
- No print/mixdown tape stage after the bus, no pan, no faders, no solo or mute.
- Changing any setting means calling `prepare()` again, which calibrates (roughly half a
  second) and must happen off the audio thread. There is no click-free parameter change.
- Processing cost has not been profiled. Each stem side runs a 4x oversampled tape channel
  and a 4x oversampled console stage.

## Try it

```
C=console_profiles/stages/british_class_a_discrete_line_1970.json
./build-tape/tape_machine_bench mix $C $C out.wav stem1.wav stem2.wav stem3.wav \
    --tape tape_profiles/machines/closed_loop_16track_2inch_1970.json --ips 15 --tape-drive 6 \
    --formula tape_profiles/formulas/high_output_1975.json --bus-drive 4
```
