add_library(ipc STATIC
    src/IpcPlaceholder.cpp
)

target_include_directories(ipc PUBLIC include)
target_compile_features(ipc PUBLIC cxx_std_20)
