add_library(morphing STATIC
    ParameterInterpolator.cpp
    AdaptiveResponseEngine.cpp
    TemporalSceneEvolution.cpp
)

target_include_directories(morphing PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(morphing PUBLIC cxx_std_20)
