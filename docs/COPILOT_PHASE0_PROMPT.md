# Copilot Phase 0 Prompt

You are building ConsoleCore, a macOS C++/JUCE standalone audio application.

Current task: make Phase 0 compile and run.

Rules:

1. Do not implement console emulation yet.
2. Do not add proprietary console model names in code.
3. Keep all DSP pass-through for now.
4. Prioritize stable build, audio device selection, meters, and Git backup.
5. Do not block or allocate on the audio thread.
6. Keep changes small and commit frequently.

Steps:

1. Verify JUCE is available in `external/JUCE`, or configure CMake with `-DJUCE_DIR=/path/to/JUCE`.
2. Run `./scripts/configure_macos.sh`.
3. Fix any CMake or JUCE API errors.
4. Run `./scripts/build_debug.sh`.
5. Launch the ConsoleCore app.
6. Confirm the Audio Settings dialog opens.
7. Confirm meters draw without crashing.
8. Run `./scripts/run_tests.sh`.
9. Commit the working state.

Expected result:

A working macOS app named ConsoleCore that opens, allows CoreAudio device selection, passes audio through, and shows basic meter telemetry.
