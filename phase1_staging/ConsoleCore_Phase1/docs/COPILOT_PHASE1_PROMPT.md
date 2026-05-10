# ConsoleCore Copilot Prompt — Phase 1

You are working in the ConsoleCore repository.

Goal: validate the Mac development environment and create the first functional standalone audio-routing milestone.

Do not implement branded console models yet. Do not copy or reference third-party JSFX code. Phase 1 is infrastructure only.

Tasks:
1. Clone JUCE into `external/JUCE` or configure CMake with `-DJUCE_DIR=/path/to/JUCE`.
2. Configure and build the standalone app on Apple Silicon macOS.
3. Confirm the app opens and displays CoreAudio device information.
4. Confirm the Audio Settings dialog can select a Loopback device.
5. Confirm pass-through audio works from Loopback input to selected output.
6. Confirm the meters respond on active channels.
7. Confirm `Save Device Preset` writes `~/Documents/ConsoleCore/ConsoleCore_DevicePreset.json`.
8. Run the smoke tests.
9. Initialize git, commit Phase 1, and push to GitHub if a remote is available.

Expected commands:

```bash
./scripts/configure_macos.sh
./scripts/build_debug.sh
./scripts/run_tests.sh
./scripts/bootstrap_git.sh
```

Acceptance criteria:
- CMake config succeeds.
- ConsoleCoreStandalone builds.
- App launches.
- Audio device selector opens.
- Loopback device can be selected.
- Meters respond to audio.
- Device preset JSON is created.
- Tests pass.

If anything fails, document the exact command, error output, macOS version, Xcode version, JUCE path, and whether the machine is Apple Silicon or Intel.
