/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibWeb/Bindings/PythonPlatform.h>
#include <LibGC/Forward.h>
#include <LibJS/Forward.h>
#include <LibWeb/Forward.h>

namespace Web::Bindings {

// Bridge for cross-language communication between Python and JavaScript
class PythonJSBridge {
public:
    // Initialize the bridge system
    static bool initialize_bridge();

    // Convert Python value to JavaScript value
    static JS::Value python_to_js(PyObject* py_obj, JS::Realm& realm);

    // Convert JavaScript value to Python value
    static PyObject* js_to_python(JS::Value js_val, JS::VM& vm);

    // Call a JavaScript function from Python
    static PyObject* call_js_function(String const& function_name, PyObject* args, JS::Realm& realm);

    // Call a Python function from JavaScript
    static JS::Value call_python_function(PyObject* func, GC::RootVector<JS::Value>& js_args, JS::Realm& realm);

    // Get JavaScript global object for cross-language access
    static PyObject* get_js_global_object();

    // Set up the bridge in Python execution environment
    static bool setup_bridge_in_context(PyObject* globals, JS::Realm& realm);

private:
    static bool s_bridge_initialized;
    static PyObject* s_js_proxy_type; // Python type for JS object proxies
    static PyObject* s_py_proxy_type; // Python type for Python function proxies
};

}
