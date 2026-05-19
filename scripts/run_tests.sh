#!/bin/bash
# Download doctest and run tests

set -e

PROJECT_DIR="$(cd "$(dirname "$0")/../" && pwd)"
TESTS_DIR="$(cd "$PROJECT_DIR/tests" && pwd)"

echo "=== SIYI SDK C++ Tests ==="

# Download doctest if not present
if [ ! -f "$TESTS_DIR/doctest.h" ]; then
    echo "Downloading doctest.h..."
    mkdir -p "$TESTS_DIR"
    wget -O "$TESTS_DIR/doctest.h" \
        https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h \
        || curl -o "$TESTS_DIR/doctest.h" \
            https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h
    echo "doctest.h downloaded"
fi

# Compile
echo "Building tests..."
cd "$PROJECT_DIR"
g++ -std=c++17 -I"$TESTS_DIR" -Iinclude \
    tests/test_base_camera.cpp \
    tests/test_zr30_camera.cpp \
    src/camera/base_camera.cpp \
    src/camera/zr30_camera.cpp \
    src/helper/log_manager.cpp \
    -o test_runner

echo "Build complete"

# Run
echo "Running tests..."
./test_runner
