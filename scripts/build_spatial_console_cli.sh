#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../tools/spatial_console_cli"
cmake -S . -B build
cmake --build build --config Release
./build/ConsoleCoreSpatialConsoleCLI