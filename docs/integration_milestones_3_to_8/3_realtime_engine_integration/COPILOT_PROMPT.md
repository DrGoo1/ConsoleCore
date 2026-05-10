# Copilot Prompt — Realtime Engine Integration

Implement the milestone incrementally.

Requirements:
- Preserve realtime safety.
- No heap allocation in the audio callback after prepare().
- No blocking locks in the DSP thread.
- Maintain deterministic output.
- Preserve compatibility with existing ConsoleCore + TapeCore integration milestones.

Priority tasks:
1. JUCE audio callback integration
2. Loopback multichannel routing
3. Realtime stem graph
4. Per-stem tape runtime
5. Integrated telemetry
6. Latency reporting
7. Runtime profile loading
8. Transport synchronization

Acceptance:
- Clean Release build
- Stable realtime processing
- No NaN/Inf output
- Validation CLI/tests pass
