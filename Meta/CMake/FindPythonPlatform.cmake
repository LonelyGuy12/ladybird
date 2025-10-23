# Platform-aware Python finder for Ladybird
# Handles differences between Windows, macOS, and Linux Python installations

if(WIN32)
    # Windows-specific Python finding
    message(STATUS "Finding Python for Windows...")
    
    # Prefer Python from PATH, then registry
    find_package(Python3 3.8 COMPONENTS Interpreter Development REQUIRED)
    
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
    
    find_package(Python3 3.8 COMPONENTS Interpreter Development REQUIRED)
    
    if(Python3_FOUND)
        message(STATUS "Python3 found: ${Python3_VERSION}")
        message(STATUS "Python3 include: ${Python3_INCLUDE_DIRS}")
        message(STATUS "Python3 libraries: ${Python3_LIBRARIES}")
        
        # Check if it's framework Python
        if(Python3_LIBRARY MATCHES "Python\\.framework")
            message(STATUS "Using Python Framework")
        else()
            message(STATUS "Using regular Python library")
        endif()
    endif()
    
else()
    # Linux and other Unix-like systems
    message(STATUS "Finding Python for Linux...")
    
    find_package(Python3 3.8 COMPONENTS Interpreter Development REQUIRED)
    
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
