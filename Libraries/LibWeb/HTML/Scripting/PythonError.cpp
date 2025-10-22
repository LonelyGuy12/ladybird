/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/HTML/Scripting/PythonError.h>
#include <Python.h>

namespace Web::HTML {

// Get Python exception information as a string
ErrorOr<String> PythonError::format_python_exception()
{
    // Python API requires that we have the GIL
    // In a real implementation, we would need to handle this properly
    
    // Get exception info
    PyObject* ptype = nullptr;
    PyObject* pvalue = nullptr;
    PyObject* ptraceback = nullptr;
    
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    if (!ptype)
        return String("Unknown Python error (no exception set)");
    
    // Format the exception as a string
    PyObject* pstr = PyObject_Str(pvalue);
    if (!pstr) {
        PyErr_Clear();
        return String("Unknown Python error (failed to get message)");
    }
    
    const char* error_msg = PyUnicode_AsUTF8(pstr);
    String result;
    
    if (error_msg) {
        result = String(error_msg);
    } else {
        result = String("Unknown Python error (failed to convert message)");
    }
    
    Py_XDECREF(pstr);
    Py_XDECREF(ptype);
    Py_XDECREF(pvalue);
    Py_XDECREF(ptraceback);
    
    PyErr_Clear();
    return result;
}

// Convert Python exception to Error object
Error PythonError::from_python_exception()
{
    // Get exception info
    PyObject* ptype = nullptr;
    PyObject* pvalue = nullptr;
    PyObject* ptraceback = nullptr;
    
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    if (!ptype) {
        return Error::from_string_literal("Unknown Python error (no exception set)");
    }
    
    // Get exception type name
    const char* type_name = "UnknownError";
    PyObject* ptype_name = PyObject_GetAttrString(ptype, "__name__");
    if (ptype_name) {
        type_name = PyUnicode_AsUTF8(ptype_name);
        if (!type_name)
            type_name = "UnknownError";
    }
    
    // Get exception message
    const char* error_msg = "Unknown error";
    PyObject* pstr = PyObject_Str(pvalue);
    if (pstr) {
        error_msg = PyUnicode_AsUTF8(pstr);
        if (!error_msg)
            error_msg = "Unknown error";
    }
    
    // Get traceback as string
    String traceback = "No traceback available";
    if (ptraceback) {
        // Use Python's traceback module to format the traceback
        PyObject* traceback_module = PyImport_ImportModule("traceback");
        if (traceback_module) {
            PyObject* format_exception = PyObject_GetAttrString(traceback_module, "format_exception");
            if (format_exception) {
                PyObject* args = PyTuple_Pack(3, ptype, pvalue, ptraceback);
                if (args) {
                    PyObject* tb_list = PyObject_CallObject(format_exception, args);
                    if (tb_list) {
                        PyObject* tb_str = PyObject_GetAttrString(tb_list, "join");
                        if (tb_str) {
                            PyObject* empty_str = PyUnicode_FromString("");
                            if (empty_str) {
                                PyObject* args2 = PyTuple_Pack(1, empty_str);
                                if (args2) {
                                    PyObject* traceback_str = PyObject_CallObject(tb_str, args2);
                                    if (traceback_str) {
                                        const char* tb_chars = PyUnicode_AsUTF8(traceback_str);
                                        if (tb_chars) {
                                            traceback = String(tb_chars);
                                        }
                                        Py_DECREF(traceback_str);
                                    }
                                    Py_DECREF(args2);
                                }
                                Py_DECREF(empty_str);
                            }
                            Py_DECREF(tb_str);
                        }
                        Py_DECREF(tb_list);
                    }
                    Py_DECREF(args);
                }
                Py_DECREF(format_exception);
            }
            Py_DECREF(traceback_module);
        }
    }
    
    // Clean up
    Py_XDECREF(pstr);
    Py_XDECREF(ptype_name);
    Py_XDECREF(ptype);
    Py_XDECREF(pvalue);
    Py_XDECREF(ptraceback);
    
    PyErr_Clear();
    
    // Create PythonError with extracted information
    return PythonError(String(error_msg), String(type_name), traceback);
}

} // namespace Web::HTML
