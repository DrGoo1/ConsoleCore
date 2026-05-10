#include "PerformanceProfiler.h"

void PerformanceProfiler::beginBlock()
{
    start = std::chrono::high_resolution_clock::now();
}

void PerformanceProfiler::endBlock(int)
{
    auto end = std::chrono::high_resolution_clock::now();
    lastMs = std::chrono::duration<double, std::milli>(end - start).count();
}

double PerformanceProfiler::lastBlockMs() const
{
    return lastMs;
}

double PerformanceProfiler::estimatedCpuPercent(double bufferDurationMs) const
{
    if (bufferDurationMs <= 0.0)
        return 0.0;

    return (lastMs / bufferDurationMs) * 100.0;
}