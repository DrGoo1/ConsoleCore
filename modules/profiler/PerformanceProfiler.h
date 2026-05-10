#pragma once
#include <chrono>

class PerformanceProfiler
{
public:
    void beginBlock();
    void endBlock(int samplesProcessed);

    double lastBlockMs() const;
    double estimatedCpuPercent(double bufferDurationMs) const;

private:
    std::chrono::high_resolution_clock::time_point start;
    double lastMs = 0.0;
};