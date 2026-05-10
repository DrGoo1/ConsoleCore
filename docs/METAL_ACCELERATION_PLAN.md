# Metal Acceleration Plan

## Initial CPU Path

Milestone F should remain CPU-first.

Metal acceleration should initially target:
- offline analysis
- profiling
- model fitting
- spectral extraction
- visualization

## Future Realtime GPU Targets

1. Oversampled nonlinear processing
2. Transformer/tape convolution
3. Topology graph solving
4. AI profile inference
5. Spectral adaptive control

## Recommended Strategy

- Maintain identical CPU/GPU math paths where possible
- Use GPU for parallelizable analysis first
- Avoid hard GPU dependency for basic realtime audio