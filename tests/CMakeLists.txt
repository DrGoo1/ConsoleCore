add_executable(ConsoleCoreSmokeTest smoke_test.cpp)
target_link_libraries(ConsoleCoreSmokeTest PRIVATE console_core topology model_capture runtime latency juce::juce_audio_basics)
add_test(NAME ConsoleCoreSmokeTest COMMAND ConsoleCoreSmokeTest)

# Physical tape machine model: standard library only.
add_executable(TapeMachineTests ../modules/tape_core/tests/tape_machine_tests.cpp)
target_link_libraries(TapeMachineTests PRIVATE tape_core integration)
target_compile_definitions(TapeMachineTests PRIVATE TAPE_PROFILE_DIR="${CMAKE_SOURCE_DIR}/tape_profiles/machines")
add_test(NAME TapeMachineTests COMMAND TapeMachineTests)

# Physical console stage model: standard library only.
add_executable(ConsoleStageTests ../modules/console_physical/tests/console_stage_tests.cpp)
target_link_libraries(ConsoleStageTests PRIVATE console_physical tape_core integration)
target_compile_definitions(ConsoleStageTests PRIVATE
    CONSOLE_PROFILE_DIR="${CMAKE_SOURCE_DIR}/console_profiles/stages"
    TAPE_PROFILE_DIR="${CMAKE_SOURCE_DIR}/tape_profiles/machines")
add_test(NAME ConsoleStageTests COMMAND ConsoleStageTests)

# Stem mix runtime (tape machine into console bus): standard library only.
add_executable(StemMixTests ../modules/runtime/tests/stem_mix_tests.cpp)
target_link_libraries(StemMixTests PRIVATE runtime console_physical tape_core)
target_compile_definitions(StemMixTests PRIVATE
    CONSOLE_PROFILE_DIR="${CMAKE_SOURCE_DIR}/console_profiles/stages"
    TAPE_PROFILE_DIR="${CMAKE_SOURCE_DIR}/tape_profiles/machines")
add_test(NAME StemMixTests COMMAND StemMixTests)
