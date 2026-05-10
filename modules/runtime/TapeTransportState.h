#pragma once

struct TapeTransportState
{
    bool playing = false;
    bool recording = false;

    double tapePositionSeconds = 0.0;
    double tapeSpeedIps = 15.0;

    float wowFlutterDepth = 0.0f;
    float transportStress = 0.0f;
};
