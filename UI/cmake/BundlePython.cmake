#!/usr/bin/env cmake -P

# Script to bundle Python during installation
# Called from InstallRules.cmake

if(NOT CONTENTS_DIR)
    message(FATAL_ERROR "CONTENTS_DIR must be specified")
endif()

message(STATUS "Bundling Python into app bundle...")

set(PYTHON_SRC "/opt/homebrew/opt/python@3.14/Frameworks/Python.framework")
set(PYTHON_DEST "${CONTENTS_DIR}/Resources/bundled_python")

if(NOT EXISTS "${PYTHON_SRC}")
    message(WARNING "Python framework not found at ${PYTHON_SRC}")
    return()
endif()

# Copy Python framework
message(STATUS "Copying Python framework...")
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${PYTHON_SRC}" "${PYTHON_DEST}")

# Create Python.app wrapper with recursion detection
message(STATUS "Creating Python.app wrapper...")
set(PYTHON_APP_DIR "${PYTHON_DEST}/Versions/3.14/Resources/Python.app/Contents/MacOS")
execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf "${PYTHON_DEST}/Versions/3.14/Resources/Python.app")
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${PYTHON_APP_DIR}")

file(WRITE "${PYTHON_APP_DIR}/Python" "#!/bin/bash
[ -n \"$LADYBIRD_PYTHON_APP_GUARD\" ] && exit 0
export LADYBIRD_PYTHON_APP_GUARD=1
exec \"$(cd \"$(dirname \"$0\")\" && pwd)/../../../../bin/python3.14\" \"$@\"
")
execute_process(COMMAND chmod +x "${PYTHON_APP_DIR}/Python")

# Fix dylib references
message(STATUS "Fixing Python dylib references...")
execute_process(
    COMMAND ${CMAKE_COMMAND} -DBUNDLE_DIR="${CONTENTS_DIR}" 
            -P "${CMAKE_CURRENT_LIST_DIR}/FixPythonBundle.cmake"
)

# Install pip
message(STATUS "Installing pip...")
execute_process(
    COMMAND "${PYTHON_DEST}/Versions/3.14/bin/python3.14" -m ensurepip --default-pip
    OUTPUT_QUIET ERROR_QUIET
)

message(STATUS "Python bundling complete!")
