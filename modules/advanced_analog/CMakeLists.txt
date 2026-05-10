add_library(advanced_analog STATIC
    ThermalDriftModel.cpp
    MicroInstabilityModel.cpp
    DynamicImpedanceModel.cpp
)

target_include_directories(advanced_analog PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(advanced_analog PUBLIC cxx_std_20)
