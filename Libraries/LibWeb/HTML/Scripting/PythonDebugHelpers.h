/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Debug.h>
#include <Python.h>

// Enable Python debug logging by setting PYTHON_DEBUG to 1
#ifndef PYTHON_DEBUG
#    define PYTHON_DEBUG 1
#endif

namespace Web::HTML {

// Helper to log Python engine status
inline void debug_python_status(char const* context)
{
#if PYTHON_DEBUG
    dbgln("🐍 [PythonDebug] {}", context);
    if (Py_IsInitialized()) {
        dbgln("  ✓ Python is initialized");
        dbgln("  ✓ Python version: {}", Py_GetVersion());
        
        PyGILState_STATE gil_state = PyGILState_Ensure();
        
        // Check sys.path
        PyObject* sys_module = PyImport_ImportModule("sys");
        if (sys_module) {
            PyObject* path = PyObject_GetAttrString(sys_module, "path");
            if (path) {
                PyObject* path_str = PyObject_Str(path);
                if (path_str) {
                    const char* path_cstr = PyUnicode_AsUTF8(path_str);
                    if (path_cstr)
                        dbgln("  ✓ sys.path: {}", path_cstr);
                    Py_DECREF(path_str);
                }
                Py_DECREF(path);
            }
            Py_DECREF(sys_module);
        }
        
        PyGILState_Release(gil_state);
    } else {
        dbgln("  ✗ Python is NOT initialized");
    }
#else
    (void)context;
#endif
}

// Helper to log Python errors
inline void debug_python_error(char const* context)
{
#if PYTHON_DEBUG
    dbgln("🐍 [PythonError] {}", context);
    if (PyErr_Occurred()) {
        PyObject* ptype = nullptr;
        PyObject* pvalue = nullptr;
        PyObject* ptraceback = nullptr;
        
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        
        if (pvalue) {
            PyObject* pstr = PyObject_Str(pvalue);
            if (pstr) {
                const char* error_msg = PyUnicode_AsUTF8(pstr);
                if (error_msg)
                    dbgln("  ✗ Error: {}", error_msg);
                Py_DECREF(pstr);
            }
        }
        
        if (ptype) {
            PyObject* ptype_name = PyObject_GetAttrString(ptype, "__name__");
            if (ptype_name) {
                const char* type_name = PyUnicode_AsUTF8(ptype_name);
                if (type_name)
                    dbgln("  ✗ Type: {}", type_name);
                Py_DECREF(ptype_name);
            }
        }
        
        Py_XDECREF(ptype);
        Py_XDECREF(pvalue);
        Py_XDECREF(ptraceback);
        
        PyErr_Clear();
    } else {
        dbgln("  ℹ No Python error set");
    }
#else
    (void)context;
#endif
}

// Helper to test Python execution
inline bool debug_test_python_execution()
{
#if PYTHON_DEBUG
    dbgln("🐍 [PythonTest] Testing basic Python execution...");
    
    if (!Py_IsInitialized()) {
        dbgln("  ✗ Cannot test: Python not initialized");
        return false;
    }
    
    PyGILState_STATE gil_state = PyGILState_Ensure();
    
    // Create proper dictionaries for execution context
    PyObject* main_module = PyImport_AddModule("__main__");
    if (!main_module) {
        dbgln("  ✗ Cannot get __main__ module");
        PyGILState_Release(gil_state);
        return false;
    }
    
    PyObject* global_dict = PyModule_GetDict(main_module);
    PyObject* local_dict = PyDict_New();
    
    // Test simple expression
    PyObject* result = PyRun_String("2 + 2", Py_eval_input, global_dict, local_dict);
    
    if (result) {
        long value = PyLong_AsLong(result);
        dbgln("  ✓ Python execution works! 2 + 2 = {}", value);
        Py_DECREF(result);
        Py_DECREF(local_dict);
        PyGILState_Release(gil_state);
        return value == 4;
    } else {
        dbgln("  ✗ Python execution failed!");
        debug_python_error("Test execution");
        Py_DECREF(local_dict);
        PyGILState_Release(gil_state);
        return false;
    }
#else
    return true;
#endif
}

} // namespace Web::HTML
