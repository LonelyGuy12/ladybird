# Fix bundled Python dylib references
# This script is called as a POST_BUILD step

message(STATUS "=== Python Bundle Fixer ===")
message(STATUS "Bundle dir: ${BUNDLE_DIR}")

set(PYTHON_BIN "${BUNDLE_DIR}/Resources/bundled_python/Versions/3.14/bin/python3.14")
message(STATUS "Python binary: ${PYTHON_BIN}")

if(NOT EXISTS "${PYTHON_BIN}")
    message(FATAL_ERROR "Python binary not found at: ${PYTHON_BIN}")
endif()

# Step 1: Fix dylib path FIRST (before binary patching which can corrupt it)
message(STATUS "=== Step 1: Fixing dylib install name ===")

# Get current dylib path
execute_process(
    COMMAND otool -L "${PYTHON_BIN}"
    OUTPUT_VARIABLE OTOOL_OUTPUT
    ERROR_VARIABLE OTOOL_ERROR
)

message(STATUS "otool output:\n${OTOOL_OUTPUT}")

# Try to extract any Homebrew Python path
string(REGEX MATCH "/opt/homebrew/[^\n]*/Python\\.framework/Versions/3\\.14/Python" OLD_PATH "${OTOOL_OUTPUT}")

if(OLD_PATH)
    message(STATUS "Found Homebrew path: ${OLD_PATH}")
    message(STATUS "Rewriting to: @executable_path/../Python")
    
    execute_process(
        COMMAND install_name_tool -change "${OLD_PATH}" "@executable_path/../Python" "${PYTHON_BIN}"
        RESULT_VARIABLE RESULT
        ERROR_VARIABLE TOOL_ERROR
    )
    
    if(RESULT EQUAL 0)
        message(STATUS "✅ Successfully fixed Python dylib reference")
        
        # Verify the change
        execute_process(
            COMMAND otool -L "${PYTHON_BIN}"
            OUTPUT_VARIABLE VERIFY_OUTPUT
        )
        message(STATUS "Verification:\n${VERIFY_OUTPUT}")
    else()
        message(FATAL_ERROR "Failed to fix Python dylib reference: ${TOOL_ERROR}")
    endif()
else()
    message(WARNING "No Homebrew path found in otool output - Python may already be fixed or have a different path")
endif()

message(STATUS "=== Python Bundle Fixer Complete ===")
