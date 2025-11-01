/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef LIBRARIES_LIBWEB_BINDINGS_PYTHONPLATFORM_H
#define LIBRARIES_LIBWEB_BINDINGS_PYTHONPLATFORM_H
#pragma once

// Platform-specific Python configuration
// This header handles Python C API differences across platforms

#if !defined(PYTHON_PLATFORM_WINDOWS) && !defined(PYTHON_PLATFORM_MACOS) && !defined(PYTHON_PLATFORM_LINUX)
    // Auto-detect platform if not supplied via -D macro
    #if defined(_WIN32) || defined(_WIN64)
        #define PYTHON_PLATFORM_WINDOWS
    #elif defined(__APPLE__) && defined(__MACH__)
        #define PYTHON_PLATFORM_MACOS
    #elif defined(__linux__)
        #define PYTHON_PLATFORM_LINUX
    #else
        #error "Unknown platform for Python integration"
    #endif
#endif

#if defined(PYTHON_PLATFORM_WINDOWS)
    // Windows Python configuration
    // On Windows, we need to handle Debug vs Release Python libraries
    #if defined(_DEBUG) && !defined(Py_DEBUG)
        // Using Release Python with Debug build
        #undef _DEBUG
        #include <Python.h>
        #define _DEBUG
    #else
        #include <Python.h>
    #endif
    
    // Windows needs explicit dllimport
    #ifndef PyMODINIT_FUNC
        #if defined(_MSC_VER)
            #define PyMODINIT_FUNC __declspec(dllexport) PyObject*
        #endif
    #endif
    
#elif defined(PYTHON_PLATFORM_MACOS)
    // macOS Python configuration
    #ifndef PYTHON_PLATFORM_MACOS
        #define PYTHON_PLATFORM_MACOS
    #endif
    
    // Try framework Python first, fall back to regular
    #if __has_include(<Python/Python.h>)
        #include <Python/Python.h>
    #else
        #include <Python.h>
    #endif
    
#else
    // Linux and other Unix-like systems
    #ifndef PYTHON_PLATFORM_LINUX
        #define PYTHON_PLATFORM_LINUX
    #endif
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

#endif // LIBRARIES_LIBWEB_BINDINGS_PYTHONPLATFORM_H
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
