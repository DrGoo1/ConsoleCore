add_library(topology STATIC
    TopologyGraph.cpp
)

target_include_directories(topology PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(topology PUBLIC cxx_std_20)
