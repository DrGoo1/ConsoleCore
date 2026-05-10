# ConsoleCore Capture Protocol

## Purpose

This protocol defines how to capture real-world console behavior for ConsoleCore model fitting.

## Recommended Capture Signals

1. 20 Hz–20 kHz logarithmic sweep
2. 1 kHz sine at stepped levels: -30, -24, -18, -12, -6, -3 dBFS
3. Multitone dense musical stress signal
4. Impulse response
5. Step response
6. L/R crosstalk probe
7. Low-frequency saturation probe
8. Transient burst train
9. Recovery test: loud burst followed by decaying low-level signal

## Capture Rules

- Use calibrated converter gain.
- Document sample rate and bit depth.
- Capture dry source and processed return.
- Avoid clipping converters unless intentionally testing overload.
- Record routing notes and hardware chain.
- Store all WAVs with session JSON metadata.

## Folder Layout

```text
captures/
  SessionName/
    capture_session.json
    dry/
      sweep.wav
      multitone.wav
      impulse.wav
    wet/
      sweep_return.wav
      multitone_return.wav
      impulse_return.wav
    analysis/
      extracted_features.json
      fitted_profile.json
```