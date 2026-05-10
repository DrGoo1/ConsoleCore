# Copilot Prompt — ConsoleCore Milestone E

Implement ConsoleCore Milestone E: DAW Integration & Runtime Control.

Tasks:
1. Add runtime, routing, latency, transport, and automation modules to the repo.
2. Integrate routing profile selection into the JUCE standalone app.
3. Add profile load/save UI or command hooks.
4. Add latency calibration model and display.
5. Add transport sync state object and connect to OSC scaffold where available.
6. Add snapshot recall model that maps snapshot parameters to AnalogPhysicsParameters.
7. Add Loopback routing profiles under routing_profiles.
8. Keep audio routing stable and avoid changing Milestone C DSP behavior unless needed for integration.
9. Add the runtime_profile_cli test target.

Acceptance:
- Runtime CLI builds and writes test JSON files.
- JUCE app can load/select a routing profile.
- Runtime snapshots can be recalled and mapped to console parameters.
- Latency profile can be saved/loaded.
- Documentation clearly explains LUNA + Loopback workflow.