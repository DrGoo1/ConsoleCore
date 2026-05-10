# Copilot Prompt — Sonic Refinement & Measurement

Implement the milestone incrementally.

Requirements:
- Preserve realtime safety.
- No heap allocation in the audio callback after prepare().
- No blocking locks in the DSP thread.
- Maintain deterministic output.
- Preserve compatibility with existing ConsoleCore + TapeCore integration milestones.

Priority tasks:
1. Hardware comparisons
2. Capture validation
3. Tape machine profiling
4. Console comparison
5. Hysteresis refinement
6. Wow/flutter realism
7. Transient tuning
8. Measurement regression tests

Acceptance:
- Clean Release build
- Stable realtime processing
- No NaN/Inf output
- Validation CLI/tests pass
