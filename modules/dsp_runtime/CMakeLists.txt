add_library(dsp_runtime STATIC
    OversamplingManager.cpp
    MultibandPhysicsRouter.cpp
)

target_include_directories(dsp_runtime PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(dsp_runtime PUBLIC
    analog_physics
)

target_compile_features(dsp_runtime PUBLIC cxx_std_20)
