cmake_minimum_required(VERSION 3.20)
project(ConsoleCoreTapeMachineBench LANGUAGES CXX)

# Standalone build of the physical tape model, its bench tool and its tests.
# Needs no JUCE, so it can be built and run on any machine with a C++20 compiler.

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(TAPE_CORE ../../modules/tape_core)
set(TAPE_SOURCES
    ${TAPE_CORE}/PhysicalTapeMachine.cpp
    ${TAPE_CORE}/TapeSpecJson.cpp
    ${TAPE_CORE}/TapeBench.cpp
    ${TAPE_CORE}/TapeTrackProcessor.cpp
)

add_executable(tape_machine_bench main.cpp ${TAPE_SOURCES})
target_include_directories(tape_machine_bench PRIVATE ${TAPE_CORE})

add_executable(tape_machine_tests ${TAPE_CORE}/tests/tape_machine_tests.cpp ${TAPE_SOURCES})
target_include_directories(tape_machine_tests PRIVATE ${TAPE_CORE})
target_compile_definitions(tape_machine_tests PRIVATE TAPE_PROFILE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../tape_profiles/machines")

enable_testing()
add_test(NAME tape_machine_tests COMMAND tape_machine_tests)
