#!/usr/bin/env cmake -P

# Script to fix Python bundle dylib references
# Called during POST_BUILD of the Ladybird app bundle

if(NOT BUNDLE_DIR)
    message(FATAL_ERROR "BUNDLE_DIR must be specified")
endif()

message(STATUS "=== Fixing Python Bundle ===")
message(STATUS "Bundle directory: ${BUNDLE_DIR}")

set(PYTHON_BIN "${BUNDLE_DIR}/Resources/bundled_python/Versions/3.14/bin/python3.14")

if(NOT EXISTS "${PYTHON_BIN}")
    message(FATAL_ERROR "Python binary not found at: ${PYTHON_BIN}")
endif()

# Step 1: Detect the current Homebrew Python path using otool
message(STATUS "=== Step 1: Detecting Homebrew Python path ===")

execute_process(
    COMMAND otool -L "${PYTHON_BIN}"
    OUTPUT_VARIABLE OTOOL_OUTPUT
    RESULT_VARIABLE OTOOL_RESULT
)

if(NOT OTOOL_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to run otool on ${PYTHON_BIN}")
endif()

message(STATUS "otool output:\n${OTOOL_OUTPUT}")

# Extract the Homebrew path - matches /opt/homebrew/...Python
string(REGEX MATCH "/opt/homebrew/[^\n]*Python\\.framework/Versions/[0-9.]+/Python" OLD_PATH "${OTOOL_OUTPUT}")

if(OLD_PATH)
    message(STATUS "Found Homebrew path: ${OLD_PATH}")
    message(STATUS "Rewriting to: @executable_path/../Python")
    
    execute_process(
        COMMAND install_name_tool -change "${OLD_PATH}" "@executable_path/../Python" "${PYTHON_BIN}"
        RESULT_VARIABLE RESULT
        ERROR_VARIABLE TOOL_ERROR
    )
    
    if(RESULT EQUAL 0)
        message(STATUS "✅ Successfully fixed Python dylib reference in python3.14")
    else()
        message(FATAL_ERROR "Failed to fix Python dylib reference: ${TOOL_ERROR}")
    endif()
else()
    message(WARNING "No Homebrew path found in otool output - Python may already be fixed or have a different path")
endif()

message(STATUS "=== Python Bundle Fixer Complete ===")
