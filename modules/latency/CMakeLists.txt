add_library(latency STATIC
    LatencyCalibrationIO.cpp
)

target_include_directories(latency PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(latency PUBLIC cxx_std_20)
