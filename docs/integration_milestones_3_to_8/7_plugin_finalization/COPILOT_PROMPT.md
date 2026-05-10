# Copilot Prompt — Plugin Finalization & DAW Workflow

Implement the milestone incrementally.

Requirements:
- Preserve realtime safety.
- No heap allocation in the audio callback after prepare().
- No blocking locks in the DSP thread.
- Maintain deterministic output.
- Preserve compatibility with existing ConsoleCore + TapeCore integration milestones.

Priority tasks:
1. AU Sender
2. AU Return
3. Control plugin
4. Latency compensation
5. Session recall
6. Automation-safe parameters
7. LUNA workflow templates
8. Loopback presets

Acceptance:
- Clean Release build
- Stable realtime processing
- No NaN/Inf output
- Validation CLI/tests pass
