cmake_minimum_required(VERSION 3.20)
project(ConsoleCoreProductionDspCLI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ConsoleCoreProductionDspCLI
    main.cpp
    ../../modules/integration/ProductionDspEngine.cpp
)

target_include_directories(ConsoleCoreProductionDspCLI PRIVATE
    ../../modules/integration
)