cmake_minimum_required(VERSION 3.20)
project(ConsoleCoreControlSurfaceCLI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ConsoleCoreControlSurfaceCLI
    main.cpp
)

target_include_directories(ConsoleCoreControlSurfaceCLI PRIVATE
    ../../modules/control
)