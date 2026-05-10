#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../tools/realtime_validation_cli"
cmake -S . -B build
cmake --build build --config Release
./build/ConsoleCoreRealtimeValidationCLI
