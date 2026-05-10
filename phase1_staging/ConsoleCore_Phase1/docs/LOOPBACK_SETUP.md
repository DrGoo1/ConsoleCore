# Loopback Setup for ConsoleCore Phase 1

Create a Loopback virtual device for initial testing.

Recommended Phase 1 device:

```text
Name: ConsoleCore 16x2
Sources: Pass-Thru only
Channels: 16 input channels, stereo monitor return where applicable
```

Use your DAW output as the source feeding the Loopback device. ConsoleCore reads from the Loopback device through CoreAudio and sends audio to the selected output device.

For LUNA testing, keep the first pass simple:

```text
LUNA stereo output -> Loopback ConsoleCore 16x2 -> ConsoleCoreStandalone -> Apollo/monitor output
```

Do not attempt full multi-stem routing until Phase 2 or later. Phase 1 is only to verify that macOS/CoreAudio/Loopback/JUCE are functioning reliably.
