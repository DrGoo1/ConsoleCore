add_library(spatial STATIC
    ImmersiveBusManager.cpp
    SpatialCrosstalkEngine.cpp
)

target_include_directories(spatial PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(spatial PUBLIC cxx_std_20)
