#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build_linux_gxx"
BUILD_TYPE="${BUILD_TYPE:-Debug}"
PYTHON_EXECUTABLE="${PYTHON_EXECUTABLE:-/usr/local/bin/python3.13}"

BUILD_TESTS=OFF
BUILD_EXAMPLES=OFF
BUILD_BINDINGS=OFF
BUILD_CLI=ON
CLEAN_CONFIG=OFF

usage() {
  cat <<EOF
Usage:
  $(basename "$0") [options] [components...]

Components:
  test        Enable tests
  example     Enable examples
  bind        Enable python bindings
  cli         Enable cli

Options:
  --clean     Remove CMakeCache.txt and CMakeFiles before configure
  -h, --help  Show this help

Examples:
  ./scripts/build_linux_gxx.sh
  ./scripts/build_linux_gxx.sh test
  ./scripts/build_linux_gxx.sh bind example
  ./scripts/build_linux_gxx.sh test bind example
  BUILD_TYPE=Release ./scripts/build_linux_gxx.sh bind
EOF
}

for arg in "$@"; do
  case "$arg" in
    test)
      BUILD_TESTS=ON
      ;;
    example)
      BUILD_EXAMPLES=ON
      ;;
    bind)
      BUILD_BINDINGS=ON
      ;;
    cli)
      BUILD_CLI=ON
      ;;
    --clean)
      CLEAN_CONFIG=ON
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "[ERROR] Unknown argument: $arg"
      usage
      exit 1
      ;;
  esac
done

echo "[INFO] Project root   : ${PROJECT_ROOT}"
echo "[INFO] Build dir      : ${BUILD_DIR}"
echo "[INFO] Build type     : ${BUILD_TYPE}"
echo "[INFO] Python exe     : ${PYTHON_EXECUTABLE}"
echo "[INFO] Build CLI      : ${BUILD_CLI}"
echo "[INFO] Build tests    : ${BUILD_TESTS}"
echo "[INFO] Build examples : ${BUILD_EXAMPLES}"
echo "[INFO] Build bindings : ${BUILD_BINDINGS}"

mkdir -p "${BUILD_DIR}"

if [[ "${CLEAN_CONFIG}" == "ON" ]]; then
  echo "[INFO] Cleaning CMake cache only..."
  rm -f "${BUILD_DIR}/CMakeCache.txt"
  rm -rf "${BUILD_DIR}/CMakeFiles"
fi

echo "[INFO] Configuring with CMake (Unix Makefiles)..."
cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" \
  -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DPython_EXECUTABLE="${PYTHON_EXECUTABLE}" \
  -DSKYMAPF_BUILD_CLI="${BUILD_CLI}" \
  -DSKYMAPF_BUILD_TESTS="${BUILD_TESTS}" \
  -DSKYMAPF_BUILD_EXAMPLES="${BUILD_EXAMPLES}" \
  -DSKYMAPF_BUILD_PYTHON_BINDINGS="${BUILD_BINDINGS}"

echo "[INFO] Building..."
cmake --build "${BUILD_DIR}" -j"$(nproc)"

echo "[INFO] Done."