add_library(network STATIC
    ClusterTopology.cpp
    CapabilityRegistry.cpp
    RemoteTelemetryAggregator.cpp
)

target_include_directories(network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(network PUBLIC cxx_std_20)
