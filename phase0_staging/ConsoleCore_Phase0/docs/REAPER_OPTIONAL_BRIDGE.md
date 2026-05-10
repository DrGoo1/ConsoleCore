# Optional REAPER Bridge

REAPER is not the product foundation. It is an optional routing/prototyping host.

Possible uses:

- hidden routing project
- OSC-controlled mixer state
- experimental JSFX prototypes
- visualization of stem topology
- offline render comparison

ConsoleCore's commercial DSP must remain original C++/JUCE code.

## Phase 4 Concept

ConsoleCore can send OSC messages to REAPER for:

- track mute/solo
- stem labels
- project template loading
- transport/debug sync

REAPER can return state via OSC/ReaScript logging.
