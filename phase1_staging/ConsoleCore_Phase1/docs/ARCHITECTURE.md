# ConsoleCore Architecture

## Mission

ConsoleCore is a standalone macOS audio engine that receives multichannel audio from any DAW via Loopback/CoreAudio, processes it through a topology-aware console model, and returns stereo or multichannel output.

## Phase Architecture

### Phase 0

Environment and repository foundation.

- CMake + JUCE
- Standalone app shell
- Audio device selector
- Meter telemetry placeholder
- Git bootstrap
- GitHub Actions placeholder

### Phase 1

Stable Loopback pass-through.

- Multichannel input/output routing
- Explicit matrix routing
- Device preset save/load
- Roundtrip latency measurement
- Click/pulse alignment tool

### Phase 2

Topology engine.

- Channel strips
- Stem groups
- Bus graph
- Shared rail model
- Temporal state memory
- Crosstalk matrix
- Nonlinear headroom

### Phase 3

Console-family model library.

Models must use legally-safe descriptive names unless licensed or measured hardware data is available.

### Phase 4

Optional REAPER bridge.

- OSC control
- ReaScript project templates
- Experimental JSFX host layer
- Debug visualization

### Phase 5

DAW bridge plugins.

- ConsoleCore Sender AU/VST3
- ConsoleCore Return AU/VST3
- ConsoleCore Control AU/VST3

## Runtime Topology

```text
DAW stems
  -> Loopback multichannel device
  -> ConsoleCore standalone app
  -> topology-aware processing engine
  -> Loopback return or physical output
  -> DAW aux return / monitor path
```

## Design Constraints

- The audio thread must not allocate memory.
- The audio thread must not block on locks or filesystem I/O.
- Shared state must use atomics, double-buffered snapshots, or lock-free queues.
- DSP models must be deterministic at fixed sample rate and buffer size.
- Presets must be JSON and versioned.
