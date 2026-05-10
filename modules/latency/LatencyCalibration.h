#pragma once
#include <string>

struct LatencyCalibration
{
    std::string profileName;
    std::string dawName;
    int sampleRate = 48000;
    int bufferSize = 256;
    int measuredRoundtripSamples = 0;
    int manualOffsetSamples = 0;

    int compensatedLatencySamples() const
    {
        return measuredRoundtripSamples + manualOffsetSamples;
    }
};