# ConsoleCore Milestone C

ConsoleCore is a macOS console-emulation infrastructure for routing audio from any DAW through Loopback into a modular console engine.

Milestone C consolidates the next major DSP workstream into one integrated drop.

This milestone adds:

- Distributed rail physics engine
- Temporal analog memory model
- Transformer/tape hysteresis scaffolding
- Dynamic crosstalk matrix
- Spectral load analyzer
- Adaptive nonlinear headroom
- Physics parameter schema
- Offline DSP test harness

This milestone intentionally avoids exact console cloning and uses legally-safe behavior-family profiles.

## Immediate Copilot task

Open `docs/COPILOT_MILESTONE_C_PROMPT.md` and apply it to the current codebase.

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