add_library(routing STATIC
    src/RoutingMatrix.cpp
)

target_include_directories(routing PUBLIC include)
target_compile_features(routing PUBLIC cxx_std_20)
