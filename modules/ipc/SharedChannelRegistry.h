#pragma once
#include <atomic>

struct SharedChannelRegistry
{
    std::atomic<int> activeChannels {0};
    std::atomic<float> globalEnergy {0.0f};
};