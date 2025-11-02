# Platform-aware Python finder for Ladybird
# Handles differences between Windows, macOS, and Linux Python installations

if(WIN32)
    # Windows-specific Python finding
    message(STATUS "Finding Python for Windows...")
    
    # Prefer Python from PATH, then registry
    find_package(Python3 3.8 COMPONENTS Interpreter Development)
    
    if(NOT Python3_FOUND)
        message(FATAL_ERROR "Python3 not found on Windows!")
    endif()
    
    if(NOT Python3_Development_FOUND)
        message(FATAL_ERROR "Python3 Development component not found on Windows!")
        message(FATAL_ERROR "Please install Python with development headers from python.org")
    endif()
    
    # On Windows, ensure we have both .lib and .dll
    if(Python3_FOUND)
        message(STATUS "Python3 found: ${Python3_VERSION}")
        message(STATUS "Python3 include: ${Python3_INCLUDE_DIRS}")
        message(STATUS "Python3 libraries: ${Python3_LIBRARIES}")
        
        # Check for debug vs release libraries
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            if(NOT Python3_LIBRARY_DEBUG)
                message(WARNING "Debug Python library not found, using release version")
            endif()
        endif()
    endif()
    
elseif(APPLE)
    # macOS-specific Python finding
    message(STATUS "Finding Python for macOS...")
    
    # On macOS, prefer Homebrew Python over system Python
    set(Python3_FIND_FRAMEWORK LAST)
    set(Python3_FIND_STRATEGY LOCATION)
    
    # Check for Homebrew Python installations
    set(HOMEBREW_PYTHON_PATHS
        "/opt/homebrew/opt/python@3.14"
        "/opt/homebrew/opt/python@3.13"
        "/opt/homebrew/opt/python@3.12"
        "/opt/homebrew/opt/python@3.11"
        "/opt/homebrew/opt/python@3.10"
        "/usr/local/opt/python@3.14"
        "/usr/local/opt/python@3.13"
        "/usr/local/opt/python@3.12"
        "/usr/local/opt/python@3.11"
        "/usr/local/opt/python@3.10"
    )
    
    # Try to find Homebrew Python
    foreach(PYTHON_PATH ${HOMEBREW_PYTHON_PATHS})
        if(EXISTS "${PYTHON_PATH}")
            message(STATUS "Found Homebrew Python at: ${PYTHON_PATH}")
            set(Python3_ROOT_DIR "${PYTHON_PATH}")
            
            # Check for framework structure
            if(EXISTS "${PYTHON_PATH}/Frameworks/Python.framework")
                message(STATUS "  Python framework exists at: ${PYTHON_PATH}/Frameworks/Python.framework")
            endif()
            
            break()
        endif()
    endforeach()
    
    message(STATUS "Searching for Python with Python3_ROOT_DIR=${Python3_ROOT_DIR}")
    
    # Find Python - REQUIRED means the package must exist, but components can still be optional
    find_package(Python3 3.8 COMPONENTS Interpreter Development)
    
    if(NOT Python3_FOUND)
        message(FATAL_ERROR "Python3 not found! Searched in: ${Python3_ROOT_DIR}")
    endif()
    
    # Explicitly check for Development component - this is REQUIRED for compilation
    if(NOT Python3_Development_FOUND)
        message(FATAL_ERROR "Python3 Development component (headers/libraries) not found!")
        message(FATAL_ERROR "  Searched in: ${Python3_ROOT_DIR}")
        message(FATAL_ERROR "  Python3_EXECUTABLE: ${Python3_EXECUTABLE}")
        message(FATAL_ERROR "  Python3_INCLUDE_DIRS: ${Python3_INCLUDE_DIRS}")
        message(FATAL_ERROR "  Python3_LIBRARIES: ${Python3_LIBRARIES}")
        message(FATAL_ERROR "  Python3_LIBRARY_DIRS: ${Python3_LIBRARY_DIRS}")
        message(FATAL_ERROR "")
        message(FATAL_ERROR "Please install Python development headers:")
        message(FATAL_ERROR "  brew install python@3.14")
        message(FATAL_ERROR "  brew link --overwrite python@3.14")
    endif()
    
    if(Python3_FOUND)
        message(STATUS "Python3 found: ${Python3_VERSION}")
        message(STATUS "Python3 executable: ${Python3_EXECUTABLE}")
        message(STATUS "Python3 include: ${Python3_INCLUDE_DIRS}")
        message(STATUS "Python3 libraries: ${Python3_LIBRARIES}")
        message(STATUS "Python3_LIBRARY (singular): ${Python3_LIBRARY}")
        message(STATUS "Python3_LIBRARIES (plural): ${Python3_LIBRARIES}")
        
        # Check if it's framework Python and fix the library path
        # Check both Python3_LIBRARY (singular) and Python3_LIBRARIES (plural)
        set(PYTHON_LIB_TO_CHECK "${Python3_LIBRARY}")
        if(NOT PYTHON_LIB_TO_CHECK)
            set(PYTHON_LIB_TO_CHECK "${Python3_LIBRARIES}")
        endif()
        
        if(PYTHON_LIB_TO_CHECK MATCHES "Python\\.framework")
            message(STATUS "Using Python Framework")
            # Extract framework path and use the framework binary instead of dylib
            string(REGEX REPLACE "/lib/libpython.*\\.dylib$" "/Python" PYTHON_FRAMEWORK_BINARY "${PYTHON_LIB_TO_CHECK}")
            if(EXISTS "${PYTHON_FRAMEWORK_BINARY}")
                set(Python3_LIBRARIES "${PYTHON_FRAMEWORK_BINARY}")
                set(Python3_LIBRARY "${PYTHON_FRAMEWORK_BINARY}")
                message(STATUS "  Using framework binary: ${Python3_LIBRARIES}")
            else()
                message(WARNING "Framework binary not found at: ${PYTHON_FRAMEWORK_BINARY}")
            endif()
        else()
            message(STATUS "Using regular Python library")
            message(STATUS "  PYTHON_LIB_TO_CHECK was: ${PYTHON_LIB_TO_CHECK}")
        endif()
    endif()
    
else()
    # Linux and other Unix-like systems
    message(STATUS "Finding Python for Linux...")
    
    find_package(Python3 3.8 COMPONENTS Interpreter Development)
    
    if(NOT Python3_FOUND)
        message(FATAL_ERROR "Python3 not found on Linux!")
    endif()
    
    if(NOT Python3_Development_FOUND)
        message(FATAL_ERROR "Python3 Development component not found on Linux!")
        message(FATAL_ERROR "Please install Python development headers:")
        message(FATAL_ERROR "  Ubuntu/Debian: sudo apt install python3-dev")
        message(FATAL_ERROR "  Fedora/RHEL: sudo dnf install python3-devel")
        message(FATAL_ERROR "  Arch: sudo pacman -S python")
    endif()
    
    if(Python3_FOUND)
        message(STATUS "Python3 found: ${Python3_VERSION}")
        message(STATUS "Python3 include: ${Python3_INCLUDE_DIRS}")
        message(STATUS "Python3 libraries: ${Python3_LIBRARIES}")
    endif()
endif()

# Set common variables for all platforms
if(Python3_FOUND)
    set(PYTHON_INCLUDE_DIRS ${Python3_INCLUDE_DIRS})
    set(PYTHON_LIBRARIES ${Python3_LIBRARIES})
    set(PYTHON_VERSION ${Python3_VERSION})
    
    # Platform-specific compile definitions
    if(WIN32)
        set(PYTHON_PLATFORM_DEFINES PYTHON_PLATFORM_WINDOWS)
    elseif(APPLE)
        set(PYTHON_PLATFORM_DEFINES PYTHON_PLATFORM_MACOS)
    else()
        set(PYTHON_PLATFORM_DEFINES PYTHON_PLATFORM_LINUX)
    endif()
    
    message(STATUS "Python configuration complete for ${CMAKE_SYSTEM_NAME}")
else()
    message(FATAL_ERROR "Python 3.8+ with development headers is required")
endif()
