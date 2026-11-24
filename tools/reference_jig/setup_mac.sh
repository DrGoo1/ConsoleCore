#!/bin/bash
# Prepares the ConsoleCore reference jig analysis on a Mac.
#
#   bash tools/reference_jig/setup_mac.sh [JIG_FOLDER]
#
# Builds the tape bench, makes a Python environment with numpy inside the jig folder, and, once
# the dry test signals are in JIG_FOLDER/renders/dry, renders them through ConsoleCore's tape
# models into renders/ConsoleCore. Safe to run again: existing renders are kept.
# Default folder: /Volumes/MP 1/Claude_Files/ConsoleCore_reference_jig
set -euo pipefail

JIG="${1:-/Volumes/MP 1/Claude_Files/ConsoleCore_reference_jig}"
REPO="$(cd "$(dirname "$0")/../.." && pwd)"

if ! command -v cmake >/dev/null 2>&1; then
  echo "cmake is needed to build the ConsoleCore tape bench. Install it (for example 'brew install cmake') and run this again."
  exit 1
fi

echo "== Building the ConsoleCore tape bench"
cmake -S "$REPO/tools/tape_machine_bench" -B "$REPO/build-tape" -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build "$REPO/build-tape" --target tape_machine_bench -j 8 >/dev/null

echo "== Python environment in $JIG/.venv"
mkdir -p "$JIG/renders/dry"
if [ ! -x "$JIG/.venv/bin/python" ]; then
  python3 -m venv "$JIG/.venv"
fi
"$JIG/.venv/bin/python" -m pip install --quiet --disable-pip-version-check numpy

PY="$JIG/.venv/bin/python"
TOOL="$REPO/tools/reference_jig/analyze.py"
if ls "$JIG/renders/dry/"*.wav >/dev/null 2>&1; then
  echo "== Rendering the dry signals through ConsoleCore"
  "$PY" "$TOOL" consolecore "$JIG" --bench "$REPO/build-tape/tape_machine_bench"
else
  echo "== No dry signals yet in $JIG/renders/dry; run this script again once they are there"
fi

cat <<EOF

When the plugin renders are in $JIG/renders/<plugin>/, run:
  "$PY" "$TOOL" analyze "$JIG"
and read $JIG/report/REPORT.md
EOF
