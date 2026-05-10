# Copilot Prompt — Distributed Analog Interaction

Implement the milestone incrementally.

Requirements:
- Preserve realtime safety.
- No heap allocation in the audio callback after prepare().
- No blocking locks in the DSP thread.
- Maintain deterministic output.
- Preserve compatibility with existing ConsoleCore + TapeCore integration milestones.

Priority tasks:
1. Rail ↔ tape coupling
2. Adjacent-track interaction
3. Transformer network loading
4. Dynamic impedance
5. Low-frequency stress interaction
6. Cumulative bus stress
7. Shared tape ecosystem state
8. Inter-stem analog behavior

Acceptance:
- Clean Release build
- Stable realtime processing
- No NaN/Inf output
- Validation CLI/tests pass
