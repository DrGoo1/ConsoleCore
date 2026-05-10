#pragma once
#include <atomic>

struct TelemetryFrame
{
    float railStress = 0.0f;
    float peak = 0.0f;
    float cpuLoad = 0.0f;
};

class LockFreeTelemetry
{
public:
    void push(const TelemetryFrame& frame);
    TelemetryFrame latest() const;

private:
    std::atomic<float> railStress {0.0f};
    std::atomic<float> peak {0.0f};
    std::atomic<float> cpuLoad {0.0f};
};