cmake_minimum_required(VERSION 3.20)
project(ConsoleCoreRuntimeProfileCLI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ConsoleCoreRuntimeProfileCLI
    main.cpp
    ../../modules/runtime/RuntimeSessionController.cpp
    ../../modules/routing/LoopbackRoutingProfileIO.cpp
    ../../modules/latency/LatencyCalibrationIO.cpp
)

target_include_directories(ConsoleCoreRuntimeProfileCLI PRIVATE
    ../../modules/runtime
    ../../modules/routing
    ../../modules/latency
)