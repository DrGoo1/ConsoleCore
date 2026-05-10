# Copilot Prompt — ConsoleCore Milestone K

Implement ConsoleCore control-surface and visualization infrastructure.

Goals:
1. Add control modules.
2. Add virtual console strip state models.
3. Add meter bridge state management.
4. Add MIDI control mapping.
5. Add OSC remote-control mapping.
6. Add Stream Deck action schema.
7. Add remote-session scaffolding.
8. Preserve realtime-safe DSP architecture.

Acceptance:
- CLI builds successfully.
- Virtual strips store state.
- Meter bridge channels update correctly.
- MIDI/OSC mappings serialize/load.
- Remote sessions store connected clients.