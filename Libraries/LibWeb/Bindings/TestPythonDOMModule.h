/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibWeb/Bindings/PythonPlatform.h>
#include <LibWeb/Forward.h>

namespace Web::Bindings {

// Python module for testing DOM bindings
class TestPythonDOMModule {
public:
    static bool initialize_module();
    static PyObject* get_module();
    
private:
    static PyObject* s_module;
    static bool s_initialized;
    
    // Test functions
    static PyObject* test_document_access(PyObject* self, PyObject* args);
    static PyObject* test_element_manipulation(PyObject* self, PyObject* args);
    static PyObject* test_window_object(PyObject* self, PyObject* args);
    
    // Module methods
    static PyMethodDef s_module_methods[];
};

}