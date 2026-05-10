add_library(hardware INTERFACE)

target_include_directories(hardware INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(hardware INTERFACE cxx_std_20)
