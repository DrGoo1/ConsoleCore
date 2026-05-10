# Loopback Profile Design

ConsoleCore should not assume one universal Loopback layout.

Instead, create named routing profiles:

- LUNA 8 Stem Console
- LUNA Stereo Mix Insert
- Logic 16 Stem Console
- REAPER 32 Channel Console
- Generic 8 Stem
- Generic 16 Stem
- Generic Stereo Master

## Example

```json
{
  "profileName": "LUNA 8 Stem Console",
  "inputDeviceName": "ConsoleCore Sends",
  "outputDeviceName": "ConsoleCore Returns",
  "sampleRate": 48000,
  "bufferSize": 256,
  "reportedRoundtripLatencySamples": 1024,
  "stems": [
    { "name": "Drums", "inputL": 1, "inputR": 2, "consoleStemId": 1 },
    { "name": "Bass", "inputL": 3, "inputR": 4, "consoleStemId": 2 }
  ],
  "returns": [
    { "name": "Mix Return", "outputL": 1, "outputR": 2 }
  ]
}
```