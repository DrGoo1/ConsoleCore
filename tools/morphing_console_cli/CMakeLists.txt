cmake_minimum_required(VERSION 3.20)
project(ConsoleCoreMorphingCLI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ConsoleCoreMorphingCLI
    main.cpp
    ../../modules/morphing/ParameterInterpolator.cpp
    ../../modules/morphing/AdaptiveResponseEngine.cpp
    ../../modules/morphing/TemporalSceneEvolution.cpp
)

target_include_directories(ConsoleCoreMorphingCLI PRIVATE
    ../../modules/morphing
)