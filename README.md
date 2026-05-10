# ConsoleCore Phase 2

ConsoleCore is a macOS console-emulation infrastructure for routing audio from any DAW through Loopback into a topology-aware console engine.

Phase 2 adds the first DSP engine layer:

- topology-aware stem configuration
- shared rail sag model
- generic console profile library
- nonlinear stem summing engine
- dynamic crosstalk
- transformer/drive/stress macro parameters
- Copilot build prompt and acceptance tests

This phase intentionally avoids exact console cloning and uses legally-safe behavior-family profiles.

## Immediate Copilot task
Open `docs/COPILOT_PHASE2_PROMPT.md` and apply it to the existing Phase 1 app shell.

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
