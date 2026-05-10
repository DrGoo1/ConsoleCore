#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../tools/runtime_profile_cli"
cmake -S . -B build
cmake --build build --config Release
./build/ConsoleCoreRuntimeProfileCLI