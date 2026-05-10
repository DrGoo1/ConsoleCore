add_library(tape_core STATIC
    TapeTrackProcessor.cpp
    TapePrintProcessor.cpp
)

target_include_directories(tape_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(tape_core PUBLIC cxx_std_20)
