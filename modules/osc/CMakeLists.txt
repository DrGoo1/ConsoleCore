add_library(osc STATIC
    OscSyncManager.cpp
)

target_include_directories(osc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(osc PUBLIC cxx_std_20)
