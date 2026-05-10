#include "../../modules/performance/RealtimeSafePool.h"
#include "../../modules/performance/LockFreeTelemetry.h"
#include "../../modules/profiler/PerformanceProfiler.h"
#include "../../modules/dsp_runtime/OversamplingManager.h"
#include <iostream>
#include <thread>
#include <chrono>

int main()
{
    RealtimeSafePool pool;
    pool.initialize(1024 * 1024);

    void* block = pool.allocate(4096);

    LockFreeTelemetry telemetry;
    telemetry.push({0.42f, 0.91f, 12.5f});

    OversamplingManager os;
    os.setMode(OversamplingMode::X4);

    PerformanceProfiler profiler;

    profiler.beginBlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    profiler.endBlock(512);

    auto latest = telemetry.latest();

    std::cout << "ConsoleCore Performance Runtime CLI\n";
    std::cout << "Pool allocation valid: " << (block != nullptr ? "yes" : "no") << "\n";
    std::cout << "Oversampling factor: " << os.factor() << "x\n";
    std::cout << "Rail stress telemetry: " << latest.railStress << "\n";
    std::cout << "Peak telemetry: " << latest.peak << "\n";
    std::cout << "Estimated CPU %: "
              << profiler.estimatedCpuPercent(10.67)
              << "\n";

    return 0;
}