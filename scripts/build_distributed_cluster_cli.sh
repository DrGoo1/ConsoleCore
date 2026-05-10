#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../tools/distributed_cluster_cli"
cmake -S . -B build
cmake --build build --config Release
./build/ConsoleCoreDistributedClusterCLI