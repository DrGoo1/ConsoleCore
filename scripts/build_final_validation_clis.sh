#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

echo "Building Milestone N Production DSP CLI..."
cd "$ROOT/milestone_N_production_dsp/tools/production_dsp_cli"
cmake -S . -B build
cmake --build build --config Release
./build/ConsoleCoreProductionDspCLI

echo "Final milestone validation completed."