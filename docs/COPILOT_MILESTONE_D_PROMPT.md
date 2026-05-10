# Copilot Prompt — ConsoleCore Milestone D

Implement the ConsoleCore Milestone D modeling and capture framework.

Tasks:
1. Add model_capture modules to build.
2. Implement test signal generation for sine, sweep, impulse, step, burst, pink-ish noise, and multitone.
3. Implement CaptureSession JSON load/save.
4. Implement FeatureExtractor:
   - RMS
   - peak
   - crest factor
   - simple THD estimate from harmonic bins or Goertzel approximation
   - stereo crosstalk estimate
   - recovery envelope estimate
5. Implement ModelFitter scaffold that maps extracted features into AnalogPhysicsParameters.
6. Implement ProfileValidator that runs a generated profile through AnalogPhysicsCore and reports stability/telemetry.
7. Add command-line capture tool under tools/model_capture_cli.
8. Keep all built-in profiles legally-safe and generic.

Acceptance:
- CLI can generate test WAV metadata files or raw float dump files if WAV writer is not yet available.
- CLI can read capture_session.json.
- CLI can output fitted_profile.json.
- No proprietary console claims in code or built-in profile names.