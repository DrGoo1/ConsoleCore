add_library(render INTERFACE)

target_include_directories(render INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(render INTERFACE cxx_std_20)
