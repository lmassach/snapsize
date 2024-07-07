#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
cmake -B build "$@"
ncpu="$(nproc --all)"
cmake --build build -j "$((ncpu-1))"
cmake --install build --prefix install
