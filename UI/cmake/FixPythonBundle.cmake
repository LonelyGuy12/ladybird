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
        message(STATUS "✅ Successfully fixed Python dylib reference in python3.14")
        
        # Add rpath so python3.14 can resolve @rpath in the dylib
        execute_process(
            COMMAND install_name_tool -add_rpath "@executable_path/.." "${PYTHON_BIN}"
            RESULT_VARIABLE RPATH_RESULT
            ERROR_VARIABLE RPATH_ERROR
        )
        
        if(RPATH_RESULT EQUAL 0)
            message(STATUS "✅ Added rpath to python3.14")
        else()
            # Rpath might already exist, check
            execute_process(
                COMMAND otool -l "${PYTHON_BIN}"
                OUTPUT_VARIABLE RPATH_CHECK
            )
            if(RPATH_CHECK MATCHES "LC_RPATH")
                message(STATUS "✅ Rpath already exists in python3.14")
            else()
                message(WARNING "Failed to add rpath: ${RPATH_ERROR}")
            endif()
        endif()
        
        # Verify the changes
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

# Step 2: Fix the Python dylib's install_id (it references itself with old path)
message(STATUS "=== Step 2: Fixing Python dylib install_id ===")

set(PYTHON_DYLIB "${BUNDLE_DIR}/Resources/bundled_python/Versions/3.14/Python")

if(EXISTS "${PYTHON_DYLIB}")
    # Use simple @rpath/Python instead of @rpath/Python.framework/Versions/3.14/Python
    # The latter creates recursive path resolution with @executable_path/.. rpath
    execute_process(
        COMMAND install_name_tool -id "@rpath/Python" "${PYTHON_DYLIB}"
        RESULT_VARIABLE DYLIB_RESULT
        ERROR_VARIABLE DYLIB_ERROR
    )
    
    if(DYLIB_RESULT EQUAL 0)
        message(STATUS "✅ Successfully fixed Python dylib install_id to @rpath/Python")
    else()
        message(FATAL_ERROR "Failed to fix Python dylib install_id: ${DYLIB_ERROR}")
    endif()
else()
    message(FATAL_ERROR "Python dylib not found at: ${PYTHON_DYLIB}")
endif()

message(STATUS "=== Python Bundle Fixer Complete ===")
