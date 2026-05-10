# ConsoleCore

ConsoleCore is a commercial-safe, topology-aware console-emulation infrastructure for macOS. The target system routes audio from any Mac DAW through Loopback into a proprietary standalone processing engine, then returns the processed signal to a DAW, monitor path, or audio interface.

## Phase 1 status

Phase 1 is an environment and routing validation build.

Implemented:
- JUCE/CMake standalone app scaffold
- CoreAudio device selector
- Loopback-oriented pass-through path
- 64-channel app callback configuration
- routing matrix module
- RMS/peak meter state
- device preset JSON save scaffold
- latency probe scaffold
- Git bootstrap scripts
- Copilot Phase 1 prompt

Not implemented yet:
- console modeling
- branded console profiles
- distributed rail topology
- AU/VST bridge plugins
- REAPER bridge

## Build

Install Xcode command line tools and JUCE, then either clone JUCE into `external/JUCE` or pass `-DJUCE_DIR=/path/to/JUCE`.

```bash
./scripts/configure_macos.sh
./scripts/build_debug.sh
./scripts/run_tests.sh
```

## Git backup

```bash
./scripts/bootstrap_git.sh
```

Then add a remote manually if needed:

```bash
git remote add origin git@github.com:YOUR_ACCOUNT/ConsoleCore.git
git push -u origin main
```

## Safety/IP

ConsoleCore should model generic console behaviors from lawful public information, original measurements, and original DSP research. Do not copy third-party plugin, JSFX, or hardware documentation code.
