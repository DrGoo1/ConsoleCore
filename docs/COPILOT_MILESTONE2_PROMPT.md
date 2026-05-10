# Copilot Prompt — TapeCore Integration Milestone 2

Goals:
1. Add realtime runtime modules.
2. Add per-stem tape runtime allocation.
3. Add stem bus manager.
4. Add transport-aware tape state.
5. Add integrated realtime telemetry.
6. Add Loopback routing abstraction.
7. Integrate into JUCE audio callback.
8. Preserve realtime-safe operation.

Acceptance:
- CLI builds.
- Multiple stem tape processors run.
- Shared tape state updates.
- No NaN/Inf.
- No dynamic allocation in process callback.
