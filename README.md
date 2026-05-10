# ConsoleCore Phase 5

ConsoleCore is a macOS console-emulation infrastructure for routing audio from any DAW through Loopback into a modular console engine.

Phase 5 introduces the first Sender/Return plugin infrastructure.

New systems:

- Sender plugin scaffold
- Return plugin scaffold
- Shared state registry
- Console session manager
- Channel identity mapping

This phase intentionally avoids exact console cloning and uses legally-safe behavior-family profiles.

## Immediate Copilot task

Open `docs/COPILOT_PHASE5_PROMPT.md` and apply it to the current codebase.

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