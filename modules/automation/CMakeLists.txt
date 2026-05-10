add_library(automation INTERFACE)

target_include_directories(automation INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(automation INTERFACE cxx_std_20)
