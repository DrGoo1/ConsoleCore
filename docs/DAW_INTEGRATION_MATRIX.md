# DAW Integration Matrix

## LUNA

Recommended first workflow:
- Use Loopback as multichannel output from LUNA where possible.
- Route LUNA buses/stems to Loopback.
- ConsoleCore receives Loopback input.
- ConsoleCore returns stereo or stems to another Loopback device or physical output.

Important:
- LUNA does not expose arbitrary REAPER-style JSFX/gmem routing.
- Treat LUNA as the production DAW and ConsoleCore as external console engine.

## Logic Pro

Strong support:
- Multichannel virtual audio routing
- AU plugins
- IAC/OSC/MIDI control options

## REAPER

Best technical host:
- Arbitrary routing
- OSC
- ReaScript
- JSFX prototyping
- Hidden-host bridge potential

## Pro Tools

Possible with:
- Loopback
- Aux routing
- AAX later if licensed/developed

## Ableton Live

Possible with:
- External Audio Effect style routing
- Loopback stereo/stem returns

## Cubase/Nuendo

Possible with:
- External FX routing
- Loopback multichannel configuration