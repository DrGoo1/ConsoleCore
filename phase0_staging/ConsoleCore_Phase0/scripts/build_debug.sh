#!/usr/bin/env bash
set -euo pipefail
cmake --build build --config Debug -j "$(sysctl -n hw.ncpu)"
