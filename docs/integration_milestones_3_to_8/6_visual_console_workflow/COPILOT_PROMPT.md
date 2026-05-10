# Copilot Prompt — Visual Console & Workflow

Implement the milestone incrementally.

Requirements:
- Preserve realtime safety.
- No heap allocation in the audio callback after prepare().
- No blocking locks in the DSP thread.
- Maintain deterministic output.
- Preserve compatibility with existing ConsoleCore + TapeCore integration milestones.

Priority tasks:
1. Console strips
2. Tape reels
3. Meter bridge
4. Topology visualization
5. Profile browser
6. Routing matrix
7. Telemetry panels
8. Capture workflow UI

Acceptance:
- Clean Release build
- Stable realtime processing
- No NaN/Inf output
- Validation CLI/tests pass
