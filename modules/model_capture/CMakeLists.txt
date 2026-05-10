add_library(model_capture STATIC
    CaptureSessionIO.cpp
    TestSignalGenerator.cpp
    FeatureExtractor.cpp
    ModelFitter.cpp
    ProfileValidator.cpp
)

target_include_directories(model_capture PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(model_capture PUBLIC
    analog_physics
)

target_compile_features(model_capture PUBLIC cxx_std_20)
