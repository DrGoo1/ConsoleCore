#pragma once
#include <atomic>

struct SharedRailState
{
    std::atomic<float> railVoltage {1.0f};
    std::atomic<float> railStress {0.0f};
    std::atomic<float> thermalState {0.0f};
    std::atomic<int> activeStemCount {0};
};