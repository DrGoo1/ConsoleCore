# Realtime Checklist

## Audio Thread Rules

- No malloc/new in process callback
- No locks/mutex waits
- No file I/O
- No network I/O
- No JSON parsing
- No dynamic container resizing

## DSP Stability

- Clamp nonlinear outputs
- Sanitize NaN/Inf
- Deterministic random seeds
- Fixed oversampling latency
- Predictable transport state

## Telemetry

- Lock-free atomics only
- UI polling from non-audio thread
