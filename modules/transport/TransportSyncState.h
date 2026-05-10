#pragma once
#include <cstdint>

struct TransportSyncState
{
    bool playing = false;
    bool recording = false;
    double bpm = 120.0;
    std::int64_t positionSamples = 0;
    double positionSeconds = 0.0;
    int timeSigNumerator = 4;
    int timeSigDenominator = 4;
};