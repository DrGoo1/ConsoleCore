# Milestone E Specification

## Goal

Create the runtime-control and integration layer required for ConsoleCore to function as a DAW-connected external console engine.

## Primary Workflow

```text
Mac DAW
  ↓
Loopback multichannel send device
  ↓
ConsoleCore Standalone Engine
  ↓
Loopback stereo/stem return device
  ↓
DAW aux return or physical output
```

## Optional Advanced Workflow

```text
DAW Sender Plugins
  ↓
ConsoleCore IPC / Shared State
  ↓
ConsoleCore Engine
  ↓
DAW Return Plugin
```

## Required Systems

1. Runtime session controller
2. Loopback routing profile loader
3. Latency calibration storage
4. OSC command map
5. Transport sync state
6. Snapshot/preset automation schema
7. REAPER hidden-host control abstraction
8. AU bridge implementation specification

## Acceptance Targets

- Session profile can describe input/output device names and channel routing.
- Latency calibration can be saved/loaded per DAW/profile.
- Runtime controller can switch profiles and snapshots.
- OSC command names are standardized.
- Copilot can integrate these models into the existing JUCE app.