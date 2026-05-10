#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../tools/model_capture_cli"
cmake -S . -B build
cmake --build build --config Release
./build/ConsoleCoreModelCaptureCLI validate