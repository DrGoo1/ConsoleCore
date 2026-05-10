add_library(analog_physics STATIC
    AnalogPhysicsCore.cpp
    RailPhysicsEngine.cpp
    TemporalMemoryEngine.cpp
    TransformerHysteresisModel.cpp
    DynamicCrosstalkMatrix.cpp
    SpectralLoadAnalyzer.cpp
    AdaptiveHeadroomModel.cpp
)

target_include_directories(analog_physics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(analog_physics PUBLIC cxx_std_20)
