add_library(console_core STATIC
    src/ConsoleCoreEngine.cpp
)

target_include_directories(console_core PUBLIC include)

target_link_libraries(console_core PUBLIC juce::juce_audio_basics)

target_compile_features(console_core PUBLIC cxx_std_20)
