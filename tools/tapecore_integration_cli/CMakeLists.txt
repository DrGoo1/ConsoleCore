cmake_minimum_required(VERSION 3.20)
project(ConsoleCoreTapeCoreIntegrationCLI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ConsoleCoreTapeCoreIntegrationCLI
    main.cpp
    ../../modules/integration/IntegratedAnalogPathEngine.cpp
    ../../modules/tape_core/TapeTrackProcessor.cpp
    ../../modules/tape_core/TapePrintProcessor.cpp
)

target_include_directories(ConsoleCoreTapeCoreIntegrationCLI PRIVATE
    ../../modules/integration
    ../../modules/tape_core
)
