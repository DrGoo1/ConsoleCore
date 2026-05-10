add_library(session STATIC
    ConsoleSessionManager.cpp
)

target_include_directories(session PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(session PUBLIC cxx_std_20)
