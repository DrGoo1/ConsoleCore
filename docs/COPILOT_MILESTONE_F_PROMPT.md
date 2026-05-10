# Copilot Prompt — ConsoleCore Milestone F

Implement ConsoleCore Milestone F.

Goals:
1. Integrate performance modules into the runtime.
2. Add realtime-safe memory pool usage patterns.
3. Add lock-free telemetry transport from DSP thread to UI/control thread.
4. Add oversampling manager.
5. Add multiband physics routing analysis.
6. Add performance profiler integration into the audio callback.
7. Prepare Metal acceleration abstraction layer.
8. Maintain Apple Silicon optimization focus.
9. Preserve deterministic DSP behavior.

Acceptance:
- CLI/runtime builds successfully.
- Audio callback performs no dynamic allocation.
- Telemetry updates safely without blocking.
- Performance profiler reports block timing.
- Oversampling mode can be switched.