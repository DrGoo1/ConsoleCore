cmake_minimum_required(VERSION 3.20)
project(ConsoleCorePerformanceRuntimeCLI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ConsoleCorePerformanceRuntimeCLI
    main.cpp
    ../../modules/performance/RealtimeSafePool.cpp
    ../../modules/performance/LockFreeTelemetry.cpp
    ../../modules/profiler/PerformanceProfiler.cpp
    ../../modules/dsp_runtime/OversamplingManager.cpp
    ../../modules/dsp_runtime/MultibandPhysicsRouter.cpp
)

target_include_directories(ConsoleCorePerformanceRuntimeCLI PRIVATE
    ../../modules/performance
    ../../modules/profiler
    ../../modules/dsp_runtime
)