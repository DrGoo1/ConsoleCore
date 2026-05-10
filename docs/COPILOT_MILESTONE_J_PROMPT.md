# Copilot Prompt — ConsoleCore Milestone J

Implement hardware measurement and analog capture infrastructure.

Goals:
1. Add hardware modules.
2. Add analog capture session schema.
3. Add converter calibration profiles.
4. Add patch-chain tracking.
5. Add measurement pass abstraction.
6. Add realtime hardware telemetry scaffold.
7. Preserve compatibility with prior capture/modeling milestones.

Acceptance:
- CLI builds successfully.
- Analog capture sessions serialize/load.
- Hardware registry stores multiple devices.
- Patch chains maintain ordered routing.