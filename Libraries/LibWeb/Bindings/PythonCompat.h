/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

// Cross-platform compatibility for Python C API integration
// Handles compiler and platform-specific differences

#include <LibWeb/Bindings/PythonPlatform.h>

namespace Web::Bindings {

// Windows MSVC compatibility
#ifdef _MSC_VER
    // MSVC doesn't support designated initializers in C++ mode the same way
    #define PYTHON_METHOD_DEF(name, func, flags, doc) \
        { name, reinterpret_cast<PyCFunction>(func), flags, doc }
    
    // MSVC needs explicit casts for some Python API calls
    #define PYTHON_CAST_FUNCTION(func) reinterpret_cast<PyCFunction>(func)
    
    // Disable some MSVC warnings for Python code
    #pragma warning(push)
    #pragma warning(disable: 4100) // unreferenced formal parameter
    #pragma warning(disable: 4127) // conditional expression is constant
    #pragma warning(disable: 4244) // conversion, possible loss of data
#else
    // GCC/Clang support designated initializers properly
    #define PYTHON_METHOD_DEF(name, func, flags, doc) \
        { name, reinterpret_cast<PyCFunction>(func), flags, doc }
    
    #define PYTHON_CAST_FUNCTION(func) func
#endif

// Platform-specific PyMethodDef sentinel
#define PYTHON_METHOD_SENTINEL \
    { nullptr, nullptr, 0, nullptr }

// Platform-specific PyGetSetDef sentinel
#define PYTHON_GETSET_SENTINEL \
    { nullptr, nullptr, nullptr, nullptr, nullptr }

// Safe PyArg_ParseTuple for all platforms
template<typename... Args>
inline bool parse_python_args(PyObject* args, char const* format, Args&... arguments)
{
    return PyArg_ParseTuple(args, format, &arguments...) != 0;
}

// Safe string conversion that works on all platforms
inline char const* get_python_string(PyObject* obj)
{
    if (!PyUnicode_Check(obj))
        return nullptr;
    return PyUnicode_AsUTF8(obj);
}

// Platform-independent Python object null check
inline bool is_python_none(PyObject* obj)
{
    return obj == Py_None;
}

// Safe Python long to C++ conversion
inline bool python_to_long(PyObject* obj, long& result)
{
    if (!PyLong_Check(obj))
        return false;
    result = PyLong_AsLong(obj);
    return !PyErr_Occurred();
}

// Safe Python double to C++ conversion
inline bool python_to_double(PyObject* obj, double& result)
{
    if (!PyFloat_Check(obj))
        return false;
    result = PyFloat_AsDouble(obj);
    return !PyErr_Occurred();
}

} // namespace Web::Bindings

// Restore MSVC warnings
#ifdef _MSC_VER
    #pragma warning(pop)
#endif
