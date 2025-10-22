/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibWeb/Forward.h>
#include <Python.h>

namespace Web::Bindings {

// Python bridge to DOM objects
class PythonObjectBridge {
public:
    // Create a Python object that wraps a C++ DOM object
    static PyObject* create_wrapper_for_dom_object(void* cpp_object, const char* type_name);
    
    // Get the underlying C++ object from a Python wrapper
    static void* get_cpp_object_from_wrapper(PyObject* wrapper);
    
    // Check if a Python object is a wrapper for a C++ DOM object
    static bool is_dom_wrapper(PyObject* obj);
    
    // Initialize the Python binding system
    static void initialize_bindings();
    
    // Add all DOM API classes to the Python module
    static bool add_dom_apis_to_module(PyObject* module);
};

}