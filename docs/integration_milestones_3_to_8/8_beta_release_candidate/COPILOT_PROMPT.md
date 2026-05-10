# Copilot Prompt — Beta Release Candidate

Implement the milestone incrementally.

Requirements:
- Preserve realtime safety.
- No heap allocation in the audio callback after prepare().
- No blocking locks in the DSP thread.
- Maintain deterministic output.
- Preserve compatibility with existing ConsoleCore + TapeCore integration milestones.

Priority tasks:
1. CPU optimization
2. Crash hardening
3. Installer
4. Notarization
5. Licensing
6. Documentation
7. Beta telemetry
8. Support tooling

Acceptance:
- Clean Release build
- Stable realtime processing
- No NaN/Inf output
- Validation CLI/tests pass
