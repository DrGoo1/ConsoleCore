#pragma once
#include <vector>

struct TelemetryPoint
{
    float railStress = 0.0f;
    float headroomGain = 1.0f;
    float peak = 0.0f;
};

class TelemetryCapture
{
public:
    void push(const TelemetryPoint& p);
    const std::vector<TelemetryPoint>& points() const;

private:
    std::vector<TelemetryPoint> history;
};