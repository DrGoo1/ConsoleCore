add_library(ai STATIC
    SimilarityScorer.cpp
    AdaptiveConsoleMatcher.cpp
    DatasetManager.cpp
    TelemetryCapture.cpp
)

target_include_directories(ai PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(ai PUBLIC cxx_std_20)
