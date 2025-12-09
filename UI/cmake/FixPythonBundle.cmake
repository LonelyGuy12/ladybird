# Fix bundled Python dylib references
# This script is called as a POST_BUILD step

message(STATUS "Fixing Python bundle...")

set(PYTHON_BIN "${BUNDLE_DIR}/Resources/bundled_python/Versions/3.14/bin/python3.14")

# Step 1: Patch out hardcoded Python.app string
message(STATUS "Removing hardcoded Python.app reference...")
execute_process(
    COMMAND perl -pi -e "s|Resources/Python.app/Contents/MacOS/Python|\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00|g" "${PYTHON_BIN}"
    RESULT_VARIABLE PATCH_RESULT
)

if(PATCH_RESULT EQUAL 0)
    message(STATUS "✅ Patched Python.app string")
else()
    message(WARNING "Failed to patch Python.app string")
endif()

# Step 2: Fix dylib path
message(STATUS "Fixing dylib install name...")

# Get current dylib path
execute_process(
    COMMAND otool -L "${PYTHON_BIN}"
    OUTPUT_VARIABLE OTOOL_OUTPUT
)

# Extract Homebrew Python path
string(REGEX MATCH "/opt/homebrew/[^\n]*/Python.framework/Versions/3.14/Python" OLD_PATH "${OTOOL_OUTPUT}")

if(OLD_PATH)
    message(STATUS "Found Homebrew path: ${OLD_PATH}")
    message(STATUS "Rewriting to: @executable_path/../Python")
    
    execute_process(
        COMMAND install_name_tool -change "${OLD_PATH}" "@executable_path/../Python" "${PYTHON_BIN}"
        RESULT_VARIABLE RESULT
    )
    
    if(RESULT EQUAL 0)
        message(STATUS "✅ Successfully fixed Python dylib reference")
    else()
        message(WARNING "Failed to fix Python dylib reference")
    endif()
else()
    message(STATUS "No Homebrew path found - Python may already be fixed")
endif()
