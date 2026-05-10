cmake_minimum_required(VERSION 3.20)
project(ConsoleCoreModelCaptureCLI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ConsoleCoreModelCaptureCLI
    main.cpp
    ../../modules/model_capture/TestSignalGenerator.cpp
    ../../modules/model_capture/FeatureExtractor.cpp
    ../../modules/model_capture/ModelFitter.cpp
    ../../modules/model_capture/ProfileValidator.cpp
    ../../modules/analog_physics/AnalogPhysicsCore.cpp
    ../../modules/analog_physics/RailPhysicsEngine.cpp
    ../../modules/analog_physics/TemporalMemoryEngine.cpp
    ../../modules/analog_physics/TransformerHysteresisModel.cpp
    ../../modules/analog_physics/DynamicCrosstalkMatrix.cpp
    ../../modules/analog_physics/SpectralLoadAnalyzer.cpp
    ../../modules/analog_physics/AdaptiveHeadroomModel.cpp
)

target_include_directories(ConsoleCoreModelCaptureCLI PRIVATE
    ../../modules/model_capture
    ../../modules/analog_physics
)