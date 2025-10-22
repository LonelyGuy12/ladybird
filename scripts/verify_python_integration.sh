#!/bin/bash
# Script to verify Python integration in Ladybird

# Exit on error
set -e

echo "===== Ladybird Python Integration Verification ====="

# Check if Python is installed
if ! command -v python3 >/dev/null 2>&1; then
    echo "ERROR: Python 3 not found. Please install Python 3."
    exit 1
fi

echo "Python 3 found: $(python3 --version)"

# Check for Python development headers
if ! python3-config --cflags >/dev/null 2>&1; then
    echo "ERROR: Python development headers not found."
    echo "Please install the Python development package:"
    echo "  - Ubuntu/Debian: sudo apt install python3-dev"
    echo "  - Fedora/RHEL: sudo dnf install python3-devel"
    echo "  - macOS: brew install python"
    exit 1
fi

echo "Python development headers found."

# Check that all fixed files are in place
echo "Checking for fixed files..."

FILES_TO_CHECK=(
    "/Users/lonely/important/ladybird/Libraries/LibWeb/Bindings/PythonJSObjectWrapper.cpp.fix"
    "/Users/lonely/important/ladybird/Libraries/LibWeb/Bindings/PythonJSBridge.cpp.fix"
    "/Users/lonely/important/ladybird/Libraries/LibWeb/Bindings/PythonDOMBindings.cpp.fix"
    "/Users/lonely/important/ladybird/Libraries/LibWeb/Bindings/PythonDOMBindings.h.fix"
    "/Users/lonely/important/ladybird/Libraries/LibWeb/HTML/Scripting/IndependentPythonEngine.h.fix"
    "/Users/lonely/important/ladybird/Libraries/LibWeb/HTML/Scripting/PythonPerformanceMetrics.h"
    "/Users/lonely/important/ladybird/Libraries/LibWeb/HTML/Scripting/PythonPerformanceMetrics.cpp"
    "/Users/lonely/important/ladybird/Libraries/LibWeb/HTML/Scripting/PythonError.cpp"
    "/Users/lonely/important/ladybird/Libraries/LibWeb/DOM/Document.h.patch"
    "/Users/lonely/important/ladybird/Libraries/LibWeb/CMakeLists.txt.patch"
)

MISSING_FILES=0
for file in "${FILES_TO_CHECK[@]}"; do
    if [ ! -f "$file" ]; then
        echo "MISSING: $file"
        MISSING_FILES=$((MISSING_FILES + 1))
    else
        echo "FOUND: $file"
    fi
done

if [ $MISSING_FILES -gt 0 ]; then
    echo "ERROR: $MISSING_FILES file(s) missing. Please run setup_python_integration.sh first."
    exit 1
fi

echo "All required files are present."

# Check if the setup script has been run
if [ ! -f "/Users/lonely/important/ladybird/Build/CMakeCache.txt" ]; then
    echo "ERROR: CMake build directory not initialized."
    echo "Please run ./scripts/setup_python_integration.sh first."
    exit 1
fi

# Verify that Python is enabled in the build
if ! grep -q "ENABLE_PYTHON:BOOL=ON" "/Users/lonely/important/ladybird/Build/CMakeCache.txt"; then
    echo "ERROR: Python integration is not enabled in the build."
    echo "Please run ./scripts/setup_python_integration.sh first."
    exit 1
fi

echo "Python integration is enabled in the build."

# Check if the integration test binary exists
TEST_BINARY="/Users/lonely/important/ladybird/Build/Tests/Python/PythonIntegrationTest"
if [ ! -f "$TEST_BINARY" ]; then
    echo "Integration test binary not found. You need to build the project first."
    echo "Would you like to build it now? (y/n)"
    read -r answer
    if [[ "$answer" =~ ^[Yy]$ ]]; then
        echo "Building the project..."
        (cd "/Users/lonely/important/ladybird/Build" && make)
    else
        echo "Skipping build."
        exit 0
    fi
fi

# Run the integration test if it exists
if [ -f "$TEST_BINARY" ]; then
    echo "Running integration tests..."
    "$TEST_BINARY"
    echo "Integration tests completed."
else
    echo "Integration test binary still not found after building."
    echo "There may be build errors."
    exit 1
fi

echo "===== Verification Complete ====="
echo "Python integration appears to be set up correctly."
echo "You can now run Ladybird with Python support:"
echo "  ./Build/Ladybird/ladybird python_demo.html"
