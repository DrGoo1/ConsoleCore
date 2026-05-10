add_library(routing STATIC
    src/RoutingMatrix.cpp
    LoopbackRoutingProfileIO.cpp
)

target_include_directories(routing PUBLIC include)
target_link_libraries(routing PUBLIC juce::juce_audio_basics)
target_compile_features(routing PUBLIC cxx_std_20)
