add_library(runtime STATIC
    RuntimeSessionController.cpp
    StemBusManager.cpp
    PerStemTapeRuntime.cpp
    PhysicalStemMixRuntime.cpp
)

target_include_directories(runtime PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(runtime PUBLIC tape_core console_physical)
target_compile_features(runtime PUBLIC cxx_std_20)
