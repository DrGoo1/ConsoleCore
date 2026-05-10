cmake_minimum_required(VERSION 3.22)

project(ConsoleCore VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(CONSOLECORE_BUILD_TESTS "Build ConsoleCore tests" ON)

# JUCE location strategy:
# 1. Prefer external/JUCE inside this repository.
# 2. Otherwise allow -DJUCE_DIR=/path/to/JUCE from the command line.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE/CMakeLists.txt")
    add_subdirectory(external/JUCE)
elseif(DEFINED JUCE_DIR AND EXISTS "${JUCE_DIR}/CMakeLists.txt")
    add_subdirectory(${JUCE_DIR} JUCE)
else()
    message(FATAL_ERROR "JUCE not found. Clone JUCE into external/JUCE or configure with -DJUCE_DIR=/path/to/JUCE")
endif()

add_subdirectory(modules/console_core)
add_subdirectory(modules/topology)
add_subdirectory(modules/analog_physics)
add_subdirectory(modules/model_capture)
add_subdirectory(modules/routing)
add_subdirectory(modules/meters)
add_subdirectory(modules/ipc)
add_subdirectory(modules/session)
add_subdirectory(modules/runtime)
add_subdirectory(modules/latency)
add_subdirectory(modules/transport)
add_subdirectory(modules/automation)
add_subdirectory(modules/performance)
add_subdirectory(modules/profiler)
add_subdirectory(modules/dsp_runtime)
add_subdirectory(modules/render)
add_subdirectory(modules/ai)
add_subdirectory(modules/spatial)
add_subdirectory(modules/product)
add_subdirectory(modules/hardware)
add_subdirectory(modules/control)
add_subdirectory(modules/network)
add_subdirectory(modules/morphing)
add_subdirectory(modules/integration)
add_subdirectory(modules/ui)
add_subdirectory(modules/advanced_analog)
add_subdirectory(modules/tape_core)
add_subdirectory(modules/osc)
add_subdirectory(modules/reaper_bridge)
add_subdirectory(apps/ConsoleCoreStandalone)
add_subdirectory(tools/realtime_validation_cli)

if(CONSOLECORE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()












