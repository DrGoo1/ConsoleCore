add_library(transport INTERFACE)

target_include_directories(transport INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(transport INTERFACE cxx_std_20)
