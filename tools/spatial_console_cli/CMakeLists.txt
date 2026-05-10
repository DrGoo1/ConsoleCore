cmake_minimum_required(VERSION 3.20)
project(ConsoleCoreSpatialConsoleCLI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ConsoleCoreSpatialConsoleCLI
    main.cpp
    ../../modules/spatial/ImmersiveBusManager.cpp
    ../../modules/spatial/SpatialCrosstalkEngine.cpp
)

target_include_directories(ConsoleCoreSpatialConsoleCLI PRIVATE
    ../../modules/spatial
)