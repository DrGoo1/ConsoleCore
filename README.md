# ConsoleCore Phase 4

ConsoleCore is a macOS console-emulation infrastructure for routing audio from any DAW through Loopback into a modular console engine.

Phase 4 introduces the first bridge-layer architecture between DAWs and the ConsoleCore engine.

Major additions:

- OSC transport synchronization scaffold
- REAPER bridge service abstraction
- Shared rail IPC design

This phase intentionally avoids exact console cloning and uses legally-safe behavior-family profiles.

## Immediate Copilot task

Open `docs/COPILOT_PHASE4_PROMPT.md` and apply it to the current codebase.

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