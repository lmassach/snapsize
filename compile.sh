#!/bin/bash
# Usage: ./compile.sh [additional_cmake_config_options]
# Example: ./compile.sh -DCMAKE_BUILD_TYPE=Debug
set -euo pipefail
cd "$(dirname "$0")"
cmake -B build "$@"
ncpu="$(nproc --all)"
cmake --build build -j "$((ncpu-1))"
cmake --install build --prefix install
