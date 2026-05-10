cmake_minimum_required(VERSION 3.20)
project(ConsoleCoreRealtimeValidationCLI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ConsoleCoreRealtimeValidationCLI
    main.cpp
    ../../modules/runtime/StemBusManager.cpp
    ../../modules/runtime/PerStemTapeRuntime.cpp
    ../../modules/tape_core/TapeTrackProcessor.cpp
    ../../modules/tape_core/TapePrintProcessor.cpp
)

target_include_directories(ConsoleCoreRealtimeValidationCLI PRIVATE
    ../../modules/runtime
    ../../modules/tape_core
)
