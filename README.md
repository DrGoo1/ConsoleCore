# ConsoleCore Milestone D

ConsoleCore is a macOS console-emulation infrastructure for routing audio from any DAW through Loopback into a modular console engine.

Milestone D turns the analog physics engine into a measurable, extensible console-modeling and capture platform.

This milestone adds:

- Measurement/capture protocol
- Test signal generator
- Capture session schema + I/O
- Model fitting scaffold
- Profile validation tools
- Generic console family profiles

This milestone intentionally avoids exact console cloning and uses legally-safe behavior-family profiles.

## Immediate Copilot task

Open `docs/COPILOT_MILESTONE_D_PROMPT.md` and apply it to the current codebase.

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