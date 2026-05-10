# Copilot Prompt — ConsoleCore Milestone C

Implement the ConsoleCore Milestone C analog physics core.

Repository assumptions:
- Use existing ConsoleCore JUCE/CMake architecture.
- Add the files from this package into the repo.
- Keep code C++17-compatible.
- Preserve generic/legal-safe console modeling names.
- Do not use proprietary schematics, trademarks as model identifiers, or copied JSFX code.

Tasks:
1. Integrate modules/analog_physics into the build.
2. Implement `AnalogPhysicsCore::processBlock`.
3. Ensure RailPhysicsEngine updates per block from RMS energy and active channel count.
4. Ensure TemporalMemoryEngine smooths energy/stress with attack/release.
5. Implement TransformerHysteresisModel with bounded tanh/asymmetric memory behavior.
6. Implement DynamicCrosstalkMatrix with level-dependent L/R feed, clamped to safe gain.
7. Implement SpectralLoadAnalyzer using simple low/mid/high one-pole bands or placeholder FFT-free filters.
8. Implement AdaptiveHeadroomModel to reduce gain nonlinearly as rail stress rises.
9. Add offline test harness executable under tools/offline_physics_test.
10. Add tests confirming no NaN/Inf and stable output for sine, impulse, and pink-noise-like input.

Acceptance:
- `cmake --build build` succeeds.
- Offline test prints rail stress, headroom gain, peak output, and pass/fail stability report.
- No UI required in this milestone.