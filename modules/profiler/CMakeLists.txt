add_library(profiler STATIC
    PerformanceProfiler.cpp
)

target_include_directories(profiler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(profiler PUBLIC cxx_std_20)
