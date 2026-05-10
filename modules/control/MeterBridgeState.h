#pragma once
#include <vector>

struct MeterChannel
{
    float peak = 0.0f;
    float rms = 0.0f;
};

struct MeterBridgeState
{
    std::vector<MeterChannel> channels;
};