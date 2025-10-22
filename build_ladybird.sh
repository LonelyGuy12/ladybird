#!/bin/bash

# Build script for Ladybird browser with error logging
# This script will fix remaining Python-related issues and build the project

set -e  # Exit on any error

echo "=== Ladybird Build Script ==="
echo "Starting build process..."

# Create a log file
LOG_FILE="build_$(date +%Y%m%d_%H%M%S).log"
exec > >(tee -a "$LOG_FILE") 2>&1

echo "Log file: $LOG_FILE"
echo "Build started at: $(date)"

# Navigate to project directory
cd /Users/lonely/important/ladybird

echo "=== Step 1: Fixing remaining Python-related issues ==="

# Comment out PythonScript include in HTMLScriptElement.h if it exists
if grep -q "#include <LibWeb/HTML/Scripting/PythonScript.h>" Libraries/LibWeb/HTML/HTMLScriptElement.h; then
    sed -i '' 's/#include <LibWeb\/HTML\/Scripting\/PythonScript.h>/\/\/ #include <LibWeb\/HTML\/Scripting\/PythonScript.h>  \/\/ Disabled Python support/' Libraries/LibWeb/HTML/HTMLScriptElement.h
    echo "Commented out PythonScript include in HTMLScriptElement.h"
fi

# Comment out PythonScript include in HTMLScriptElement.cpp if it exists
if grep -q "#include <LibWeb/HTML/Scripting/PythonScript.h>" Libraries/LibWeb/HTML/HTMLScriptElement.cpp; then
    sed -i '' 's/#include <LibWeb\/HTML\/Scripting\/PythonScript.h>/\/\/ #include <LibWeb\/HTML\/Scripting\/PythonScript.h>  \/\/ Disabled Python support/' Libraries/LibWeb/HTML/HTMLScriptElement.cpp
    echo "Commented out PythonScript include in HTMLScriptElement.cpp"
fi

# Comment out PythonScript include in Scripting.cpp if it exists
if grep -q "#include.*PythonScript" Libraries/LibWeb/HTML/Scripting/Scripting.cpp 2>/dev/null; then
    sed -i '' 's/#include.*PythonScript/\/\/ &  \/\/ Disabled Python support/' Libraries/LibWeb/HTML/Scripting/Scripting.cpp
    echo "Commented out PythonScript include in Scripting.cpp"
fi

echo "=== Step 2: Running CMake configuration ==="
if cmake --preset Debug; then
    echo "CMake configuration successful"
else
    echo "CMake configuration failed"
    exit 1
fi

echo "=== Step 3: Starting build process ==="
echo "Build started at: $(date)"

if cmake --build --preset Debug --parallel 4; then
    echo "Build completed successfully at: $(date)"
    echo "=== SUCCESS ==="
    exit 0
else
    echo "Build failed at: $(date)"
    echo "=== FAILED ==="
    exit 1
fi