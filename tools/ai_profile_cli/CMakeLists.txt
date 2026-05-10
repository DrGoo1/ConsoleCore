cmake_minimum_required(VERSION 3.20)
project(ConsoleCoreAIProfileCLI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ConsoleCoreAIProfileCLI
    main.cpp
    ../../modules/ai/SimilarityScorer.cpp
    ../../modules/ai/AdaptiveConsoleMatcher.cpp
    ../../modules/ai/DatasetManager.cpp
    ../../modules/ai/TelemetryCapture.cpp
)

target_include_directories(ConsoleCoreAIProfileCLI PRIVATE
    ../../modules/ai
)