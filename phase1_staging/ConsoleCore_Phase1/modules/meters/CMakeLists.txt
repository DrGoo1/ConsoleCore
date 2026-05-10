add_library(meters STATIC
    src/LevelMeterState.cpp
)

target_include_directories(meters PUBLIC include)
target_compile_features(meters PUBLIC cxx_std_20)
