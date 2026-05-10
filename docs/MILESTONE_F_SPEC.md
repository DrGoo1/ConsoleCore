# Milestone F Specification

## Goal

Prepare ConsoleCore for large-session realtime performance while preserving deterministic analog behavior.

## Major Systems

1. DSP SIMD abstraction layer
2. Metal acceleration planning
3. Lock-free telemetry transport
4. Realtime-safe allocator strategy
5. Dynamic oversampling manager
6. Multiband analog physics routing
7. Worker-thread render architecture
8. Performance profiler
9. Freeze/render subsystem planning

## Realtime Constraints

- No heap allocation in audio callback
- No blocking locks in audio thread
- Stable CPU scaling with increasing stem count
- Deterministic telemetry timing
- Oversampling latency compensation awareness

## Future GPU Opportunities

- Spectral analysis
- Neural profile inference
- Crosstalk matrix processing
- Large topology graph updates
- Adaptive tape/convolution layers