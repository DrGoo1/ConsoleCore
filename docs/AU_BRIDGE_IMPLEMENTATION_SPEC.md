# AU Bridge Implementation Specification

## Planned Plugins

### ConsoleCore Sender AU

Placed on DAW tracks or buses.

Responsibilities:
- Assign persistent stem ID
- Send audio to ConsoleCore engine or participate in shared state
- Report stem activity and energy
- Optional local pre-conditioning

### ConsoleCore Return AU

Placed on DAW aux/mix bus.

Responsibilities:
- Receive processed ConsoleCore return
- Display rail/stress telemetry
- Expose global macros
- Provide DAW automation access

### ConsoleCore Control AU

Optional non-audio control plugin.

Responsibilities:
- Load profile
- Recall snapshot
- Select console model
- Display topology graph

## IPC Options

1. Process-local shared singleton if plugins and engine live in same host
2. Shared memory ring buffer
3. Local UDP/OSC
4. CoreAudio virtual device routing
5. Hybrid Loopback + OSC control

## First Implementation Recommendation

Use Loopback for audio and OSC/local IPC for control.

Do not attempt sample-accurate interprocess audio transport until standalone Loopback routing is stable.