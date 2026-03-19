#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build_linux_gxx"
BUILD_TYPE="${BUILD_TYPE:-Debug}"
PYTHON_EXECUTABLE="/usr/local/bin/python3.13"

echo "[INFO] Project root: ${PROJECT_ROOT}"
echo "[INFO] Build dir   : ${BUILD_DIR}"
echo "[INFO] Build type  : ${BUILD_TYPE}"
echo "[INFO] Python exe  : ${PYTHON_EXECUTABLE}"

mkdir -p "${BUILD_DIR}"

echo "[INFO] Configuring with CMake (Unix Makefiles)..."
cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" \
  -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DPython_EXECUTABLE="${PYTHON_EXECUTABLE}"

echo "[INFO] Building..."
cmake --build "${BUILD_DIR}" -j"$(nproc)"

echo "[INFO] Done."