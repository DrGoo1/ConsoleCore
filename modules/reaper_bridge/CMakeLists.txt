add_library(reaper_bridge STATIC
    ReaperBridgeService.cpp
)

target_include_directories(reaper_bridge PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(reaper_bridge PUBLIC cxx_std_20)
