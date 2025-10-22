#!/usr/bin/env bash
set -euo pipefail

# Ladybird build helper for Python-enabled build
# Usage examples:
#   bash scripts/build_python.sh --install --build
#   bash scripts/build_python.sh --build --run-test
#   bash scripts/build_python.sh --clean

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
cd "$ROOT_DIR"

DO_INSTALL=0
DO_BUILD=0
DO_CLEAN=0
DO_RUN_TEST=0

for arg in "$@"; do
  case "$arg" in
    --install) DO_INSTALL=1 ;;
    --build) DO_BUILD=1 ;;
    --clean) DO_CLEAN=1 ;;
    --run-test) DO_RUN_TEST=1 ;;
    *) echo "Unknown option: $arg" >&2; exit 1 ;;
  esac
done

if [[ $DO_CLEAN -eq 1 ]]; then
  echo "[clean] Removing Build/release directory..."
  rm -rf "$ROOT_DIR/Build/release"
fi

if [[ $DO_INSTALL -eq 1 ]]; then
  echo "[install] Checking macOS command line tools..."
  if ! xcode-select -p >/dev/null 2>&1; then
    echo "[install] Installing Xcode Command Line Tools (this may prompt a dialog)..."
    xcode-select --install || true
  else
    echo "[install] Xcode Command Line Tools already installed."
  fi

  echo "[install] Ensuring Homebrew dependencies..."
  if ! command -v brew >/dev/null 2>&1; then
    echo "Homebrew not found. Please install Homebrew from https://brew.sh and re-run with --install." >&2
    exit 1
  fi

  brew update
  brew install autoconf autoconf-archive automake ccache cmake libtool nasm ninja pkg-config python || true
  # Optional modern clang for consistency with CI
  brew install llvm@20 || true
fi

if [[ $DO_BUILD -eq 1 ]]; then
  echo "[build] Starting build via Meta/ladybird.py..."
  # Prefer Homebrew LLVM if present and user wants it; otherwise, plain build
  if brew list --versions llvm@20 >/dev/null 2>&1; then
    export CC="$(brew --prefix llvm@20)/bin/clang"
    export CXX="$(brew --prefix llvm@20)/bin/clang++"
  fi

  ./Meta/ladybird.py build
fi

if [[ $DO_RUN_TEST -eq 1 ]]; then
  echo "[run] Launching Ladybird with test_python.html..."
  TEST_FILE="file://$ROOT_DIR/test_python.html"
  ./Meta/ladybird.py run ladybird "$TEST_FILE"
fi
