cmake_minimum_required(VERSION 3.20)
project(ConsoleCoreOfflinePhysicsTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ConsoleCoreOfflinePhysicsTest
    main.cpp
    ../../modules/analog_physics/AnalogPhysicsCore.cpp
    ../../modules/analog_physics/RailPhysicsEngine.cpp
    ../../modules/analog_physics/TemporalMemoryEngine.cpp
    ../../modules/analog_physics/TransformerHysteresisModel.cpp
    ../../modules/analog_physics/DynamicCrosstalkMatrix.cpp
    ../../modules/analog_physics/SpectralLoadAnalyzer.cpp
    ../../modules/analog_physics/AdaptiveHeadroomModel.cpp
)

target_include_directories(ConsoleCoreOfflinePhysicsTest PRIVATE
    ../../modules/analog_physics
)