add_library(runtime STATIC
    RuntimeSessionController.cpp
)

target_include_directories(runtime PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(runtime PUBLIC cxx_std_20)
