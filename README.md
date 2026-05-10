# ConsoleCore Milestone F

ConsoleCore is a macOS console-emulation infrastructure for routing audio from any DAW through Loopback into a modular console engine.

Milestone F transitions ConsoleCore toward production-scale realtime processing.

Key additions:

- Realtime-safe memory pools
- Lock-free telemetry transport
- Multiband analog physics routing
- Dynamic oversampling scaffold
- Performance profiler
- Background worker architecture scaffolding

This milestone intentionally avoids exact console cloning and uses legally-safe behavior-family profiles.

## Immediate Copilot task

Open `docs/COPILOT_MILESTONE_F_PROMPT.md` and apply it to the current codebase.

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

## Performance runtime CLI (standalone tool)

```bash
./scripts/build_performance_runtime_cli.sh
```