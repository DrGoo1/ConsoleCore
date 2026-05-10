add_library(integration STATIC
    ProductionDspEngine.cpp
)

target_include_directories(integration PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(integration PUBLIC cxx_std_20)
