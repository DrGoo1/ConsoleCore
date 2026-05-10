#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../tools/production_dsp_cli"
cmake -S . -B build
cmake --build build --config Release
./build/ConsoleCoreProductionDspCLI
