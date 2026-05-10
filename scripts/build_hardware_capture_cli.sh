#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../tools/hardware_capture_cli"
cmake -S . -B build
cmake --build build --config Release
./build/ConsoleCoreHardwareCaptureCLI