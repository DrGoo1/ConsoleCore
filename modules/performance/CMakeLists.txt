add_library(performance STATIC
    RealtimeSafePool.cpp
    LockFreeTelemetry.cpp
)

target_include_directories(performance PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(performance PUBLIC cxx_std_20)
