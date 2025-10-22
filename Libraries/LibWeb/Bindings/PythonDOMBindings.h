/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Python.h>
#include <LibWeb/Forward.h>

namespace Web::Bindings {

// Python wrapper for DOM Document with Python-friendly API
class PythonDocument {
public:
    static void setup_type();
    static PyObject* create_from_cpp_document(Web::DOM::Document& document);
    static Web::DOM::Document* get_cpp_document(PyObject* obj);

    static PyTypeObject s_type;
};

// Python wrapper for DOM Elements with Python-friendly API
class PythonElement {
public:
    static void setup_type();
    static PyObject* create_from_cpp_element(Web::DOM::Element& element);
    static Web::DOM::Element* get_cpp_element(PyObject* obj);

    static PyTypeObject s_type;
};

// Python wrapper for Window object
class PythonWindow {
public:
    static void setup_type();
    static PyObject* create_from_cpp_window(Web::HTML::Window& window);
    static Web::HTML::Window* get_cpp_window(PyObject* obj);

    static PyTypeObject s_type;
};

// Python DOM API module
class PythonDOMAPI {
public:
    static bool initialize_module();
    static PyObject* get_module();

    static PyObject* s_module;
    static bool s_initialized;
};

}
