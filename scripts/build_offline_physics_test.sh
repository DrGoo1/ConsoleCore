#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../tools/offline_physics_test"
cmake -S . -B build
cmake --build build --config Release
./build/ConsoleCoreOfflinePhysicsTest