/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

// Platform-specific Python configuration
// This header handles Python C API differences across platforms

#if defined(_WIN32) || defined(_WIN64)
    // Windows Python configuration
    #define PYTHON_PLATFORM_WINDOWS
    #include <Python.h>
    
    // Windows-specific definitions
    #ifdef _DEBUG
        #undef _DEBUG
        #include <Python.h>
        #define _DEBUG
    #endif
    
#elif defined(__APPLE__) && defined(__MACH__)
    // macOS Python configuration
    #define PYTHON_PLATFORM_MACOS
    
    // Try framework Python first, fall back to regular
    #if __has_include(<Python/Python.h>)
        #include <Python/Python.h>
    #else
        #include <Python.h>
    #endif
    
#else
    // Linux and other Unix-like systems
    #define PYTHON_PLATFORM_LINUX
    #include <Python.h>
#endif

// Platform-independent Python helpers
namespace Web::Bindings::PythonPlatform {

// Get platform name for debugging
inline constexpr char const* get_platform_name()
{
#if defined(PYTHON_PLATFORM_WINDOWS)
    return "Windows";
#elif defined(PYTHON_PLATFORM_MACOS)
    return "macOS";
#elif defined(PYTHON_PLATFORM_LINUX)
    return "Linux";
#else
    return "Unknown";
#endif
}

// Platform-specific Python library suffix
inline constexpr char const* get_library_suffix()
{
#if defined(PYTHON_PLATFORM_WINDOWS)
    return ".pyd";
#elif defined(PYTHON_PLATFORM_MACOS)
    return ".so";
#else
    return ".so";
#endif
}

} // namespace Web::Bindings::PythonPlatform
