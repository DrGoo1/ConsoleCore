add_executable(ConsoleCoreSmokeTest smoke_test.cpp)
target_link_libraries(ConsoleCoreSmokeTest PRIVATE console_core topology model_capture runtime latency juce::juce_audio_basics)
add_test(NAME ConsoleCoreSmokeTest COMMAND ConsoleCoreSmokeTest)

# Physical tape machine model: standard library only.
add_executable(TapeMachineTests ../modules/tape_core/tests/tape_machine_tests.cpp)
target_link_libraries(TapeMachineTests PRIVATE tape_core)
target_compile_definitions(TapeMachineTests PRIVATE TAPE_PROFILE_DIR="${CMAKE_SOURCE_DIR}/tape_profiles/machines")
add_test(NAME TapeMachineTests COMMAND TapeMachineTests)
