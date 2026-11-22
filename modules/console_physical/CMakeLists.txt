add_library(console_physical STATIC
    PhysicalConsoleStage.cpp
    ConsoleSpecJson.cpp
    ConsoleBench.cpp
)

target_include_directories(console_physical PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(console_physical PUBLIC tape_core)
target_compile_features(console_physical PUBLIC cxx_std_20)
