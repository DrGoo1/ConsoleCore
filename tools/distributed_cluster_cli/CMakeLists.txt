cmake_minimum_required(VERSION 3.20)
project(ConsoleCoreDistributedClusterCLI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ConsoleCoreDistributedClusterCLI
    main.cpp
    ../../modules/network/ClusterTopology.cpp
    ../../modules/network/CapabilityRegistry.cpp
    ../../modules/network/RemoteTelemetryAggregator.cpp
)

target_include_directories(ConsoleCoreDistributedClusterCLI PRIVATE
    ../../modules/network
)