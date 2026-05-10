# ConsoleCore Milestone E

ConsoleCore is a macOS console-emulation infrastructure for routing audio from any DAW through Loopback into a modular console engine.

Milestone E connects the growing ConsoleCore engine to practical DAW workflows.

This milestone adds:

- Runtime session controller
- Loopback routing profile schema + I/O
- Latency calibration data model
- Transport sync state model
- Preset automation/snapshot schema
- OSC command map

This milestone intentionally avoids exact console cloning and uses legally-safe behavior-family profiles.

## Immediate Copilot task

Open `docs/COPILOT_MILESTONE_E_PROMPT.md` and apply it to the current codebase.

## Clone (with JUCE submodule)

```bash
git clone https://github.com/DrGoo1/ConsoleCore.git
cd ConsoleCore
git submodule update --init --recursive
```

## Build + test (preset)

```bash
cmake --preset macos-debug
cmake --build --preset macos-debug
ctest --preset macos-debug -V
```

## Offline physics test

```bash
./scripts/build_offline_physics_test.sh
```

## Model capture CLI (standalone tool)

```bash
./scripts/build_model_capture_cli.sh
```

## Runtime profile CLI (standalone tool)

```bash
./scripts/build_runtime_profile_cli.sh
```