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
    ${TAPE_CORE}/TapePrintProcessor.cpp
    ../../modules/integration/IntegratedAnalogPathEngine.cpp
)

set(CONSOLE_PHYSICAL ../../modules/console_physical)
set(CONSOLE_SOURCES
    ${CONSOLE_PHYSICAL}/PhysicalConsoleStage.cpp
    ${CONSOLE_PHYSICAL}/PhysicalConsoleBus.cpp
    ${CONSOLE_PHYSICAL}/ConsoleSpecJson.cpp
    ${CONSOLE_PHYSICAL}/ConsoleBench.cpp
)
list(APPEND TAPE_SOURCES ${CONSOLE_SOURCES})

add_executable(tape_machine_bench main.cpp ${TAPE_SOURCES} ../../modules/runtime/PhysicalStemMixRuntime.cpp ../../modules/runtime/StemBusManager.cpp)
target_include_directories(tape_machine_bench PRIVATE ${TAPE_CORE})

add_executable(tape_machine_tests ${TAPE_CORE}/tests/tape_machine_tests.cpp ${TAPE_SOURCES})
target_include_directories(tape_machine_tests PRIVATE ${TAPE_CORE})
target_compile_definitions(tape_machine_tests PRIVATE TAPE_PROFILE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../tape_profiles/machines")

add_executable(console_stage_tests ${CONSOLE_PHYSICAL}/tests/console_stage_tests.cpp ${TAPE_SOURCES})
target_include_directories(console_stage_tests PRIVATE ${TAPE_CORE} ${CONSOLE_PHYSICAL})
target_compile_definitions(console_stage_tests PRIVATE
    CONSOLE_PROFILE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../console_profiles/stages"
    TAPE_PROFILE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../tape_profiles/machines")

set(RUNTIME ../../modules/runtime)
add_executable(stem_mix_tests ${RUNTIME}/tests/stem_mix_tests.cpp ${RUNTIME}/PhysicalStemMixRuntime.cpp ${RUNTIME}/StemBusManager.cpp ${TAPE_SOURCES})
target_include_directories(stem_mix_tests PRIVATE ${TAPE_CORE} ${CONSOLE_PHYSICAL} ${RUNTIME})
target_compile_definitions(stem_mix_tests PRIVATE
    CONSOLE_PROFILE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../console_profiles/stages"
    TAPE_PROFILE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../tape_profiles/machines")

enable_testing()
add_test(NAME stem_mix_tests COMMAND stem_mix_tests)
add_test(NAME tape_machine_tests COMMAND tape_machine_tests)
add_test(NAME console_stage_tests COMMAND console_stage_tests)
