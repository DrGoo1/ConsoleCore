add_library(product INTERFACE)

target_include_directories(product INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(product INTERFACE cxx_std_20)
