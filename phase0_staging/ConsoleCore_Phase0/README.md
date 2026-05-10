# ConsoleCore

ConsoleCore is a macOS-focused console-emulation infrastructure intended to connect any Mac DAW to a standalone topology-aware audio engine through Loopback or other CoreAudio virtual devices.

## Phase 0 Goal

Phase 0 establishes the development environment and proves that Copilot can build, run, and version-control the project.

This phase includes:

- CMake/JUCE project scaffold
- Standalone app target placeholder
- Core module layout
- Git bootstrap scripts
- Documentation for Loopback routing and legal-safe modeling boundaries
- Copilot phase prompt
- GitHub Actions placeholder workflow

Phase 0 does **not** implement final DSP, console models, or proprietary console emulation.

## Intended Development Stack

- macOS Apple Silicon or Intel Mac
- Xcode command line tools
- CMake 3.22+
- Git
- JUCE, preferably as a Git submodule or local dependency
- VS Code or Xcode with GitHub Copilot
- Loopback by Rogue Amoeba for virtual audio routing

## Quick Start

```bash
cd ConsoleCore
chmod +x scripts/*.sh
./scripts/bootstrap_git.sh
./scripts/configure_macos.sh
./scripts/build_debug.sh
```

If JUCE is not present, clone it into `external/JUCE`:

```bash
mkdir -p external
git clone https://github.com/juce-framework/JUCE.git external/JUCE
```

## Phase Discipline

Do not skip directly to console emulation. First prove:

1. Project config works.
2. Git backup works.
3. App opens.
4. Audio device selection works.
5. Loopback routing works.
6. Latency calibration works.

Then build the console engine.
