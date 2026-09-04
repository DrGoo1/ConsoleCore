cmake_minimum_required(VERSION 3.20)
project(ConsoleCoreSystemBehaviorQualificationCLI LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(ConsoleCoreSystemBehaviorQualificationCLI
    main.cpp
    ../../modules/analog_physics/AnalogPhysicsCore.cpp
    ../../modules/analog_physics/RailPhysicsEngine.cpp
    ../../modules/analog_physics/TemporalMemoryEngine.cpp
    ../../modules/analog_physics/TransformerHysteresisModel.cpp
    ../../modules/analog_physics/DynamicCrosstalkMatrix.cpp
    ../../modules/analog_physics/SpectralLoadAnalyzer.cpp
    ../../modules/analog_physics/AdaptiveHeadroomModel.cpp
)
target_include_directories(ConsoleCoreSystemBehaviorQualificationCLI PRIVATE ../../modules/analog_physics)
