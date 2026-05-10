# Copilot Phase 2 Prompt — Topology-Aware Mix Engine

Implement and validate ConsoleCore Phase 2.

## Goal
Add the first proprietary topology-aware console engine. This phase is not a brand-specific console clone. It is a legally-safe, generic framework for distributed analog-console behavior.

## Required work
1. Integrate `TopologyAwareMixEngine` into the standalone audio callback behind a feature flag or UI toggle.
2. Create a default topology with 8 stereo stems mapped to Loopback channels 1/2 through 15/16.
3. Add UI controls for:
   - Profile selection
   - Drive
   - Stress
   - Crosstalk
   - Transformer Weight
   - Engine bypass
4. Display rail sag, active stem count, input/output RMS, and peak meters.
5. Preserve Phase 1 pass-through mode.
6. Add preset serialization for topology/profile/controls.
7. Add tests for:
   - silent input remains silent
   - mono/stereo routing is stable
   - rail sag increases with higher signal demand
   - bypass output matches pass-through within tolerance
   - no NaN/Inf output at high drive/stress

## Constraints
- Do not add proprietary console trademarks into code identifiers.
- Do not copy JSFX code or third-party plugin code.
- Use generic model names only.
- Keep DSP real-time safe: no allocation, logging, locking, or file IO inside the audio callback.

## Acceptance
Build succeeds on Apple Silicon macOS with CMake/JUCE. The app can receive 16-channel Loopback input and emit a processed stereo console mix with stable meters and no crashes.
