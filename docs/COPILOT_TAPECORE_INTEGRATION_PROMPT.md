# Copilot Prompt — ConsoleCore + TapeCore Integration Milestone

Create a new branch:

```bash
git checkout -b integrate-tapecore-engine
```

Goal:
Integrate TapeCore as a source-level subsystem inside ConsoleCore.

Tasks:
1. Add `modules/tape_core`.
2. Add `TapeSharedState`.
3. Add `TapeMachineProfile`.
4. Add `TapeTrackProcessor`.
5. Add `TapePrintProcessor`.
6. Add `AnalogPathSettings`.
7. Add `IntegratedAnalogPathEngine`.
8. Wire the integrated path in this order:
   - input trim
   - optional track tape
   - existing ConsoleCore analog physics / ProductionDspEngine path
   - optional print tape
   - output trim
9. Add tape profile JSON files.
10. Add CLI stability test.
11. Do not create a separate TapeCore app.
12. Do not add UI yet except schemas/state.
13. Keep all realtime DSP bounded and deterministic.
14. No heap allocation in the audio callback after prepare.
15. No blocking locks in the audio callback.

Acceptance:
- CLI builds.
- Tape off path runs.
- Tape track on path runs.
- Print tape on path runs.
- Output has no NaN/Inf.
- Telemetry reports tape compression/bias/print stress.
