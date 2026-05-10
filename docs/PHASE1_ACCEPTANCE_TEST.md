# Phase 1 Acceptance Test

## Purpose

Phase 1 proves that ConsoleCore can function as a standalone CoreAudio routing application between a DAW and a return device using Loopback.

## Test topology

```text
DAW stereo/stem output
  -> Loopback virtual device
  -> ConsoleCoreStandalone
  -> physical interface or Loopback return
```

## Steps

1. Install Loopback and create a virtual device named `ConsoleCore 16x2`.
2. In the source DAW, set output to `ConsoleCore 16x2`.
3. Launch ConsoleCoreStandalone.
4. Open Audio Settings.
5. Select the Loopback device as input and the desired monitor/interface output.
6. Play audio from the DAW.
7. Verify meters move in ConsoleCore.
8. Verify audio reaches the output.
9. Click `Save Device Preset`.
10. Verify `~/Documents/ConsoleCore/ConsoleCore_DevicePreset.json` exists.

## Latency probe

The latency probe is scaffolding. It writes a pulse on output channel 1 and listens on input channel 1. It requires a physical or virtual feedback path from output 1 to input 1. Do not use this with speakers at high gain.

## Pass/fail

Pass requires:
- app launches without crash
- audio device selector functions
- meters respond to signal
- pass-through output is audible or recordable
- preset JSON saves successfully
