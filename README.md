# ConsoleCore Phase 3

ConsoleCore is a macOS console-emulation infrastructure for routing audio from any DAW through Loopback into a modular console engine.

Phase 3 is architecture-heavy and expands the project toward:

- Dynamic topology graph engine
- Shared rail manager
- Console profile schema
- Temporal analog memory scaffolding
- Sender/Return plugin architecture planning
- REAPER bridge abstraction layer
- OSC synchronization stubs

This phase intentionally avoids exact console cloning and uses legally-safe behavior-family profiles.

## Immediate Copilot task

Open `docs/COPILOT_PHASE3_PROMPT.md` and apply it to the current codebase.

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

## Build (scripts)

```bash
./scripts/configure_macos.sh
./scripts/build_debug.sh
./scripts/run_tests.sh
```