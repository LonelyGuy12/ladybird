/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Python.h>
#include <LibWeb/Forward.h>

namespace Web::Bindings {

// Python wrapper for JavaScript objects to enable cross-language access
class PythonJSObjectWrapper {
public:
    // Create a Python object that wraps a JavaScript object
    static PyObject* create_wrapper(JS::Object& js_obj);
    
    // Get the underlying JavaScript object from a Python wrapper
    static JS::Object* get_js_object(PyObject* wrapper);
    
    // Setup the Python type for JavaScript object wrappers
    static void setup_js_object_wrapper_type();
    
private:
    // Python type object for JS wrappers
    static PyTypeObject s_js_object_wrapper_type;
    
    // Python type structure
    typedef struct {
        PyObject_HEAD
        void* js_object_ptr;  // Will point to JS::Object*
    } JSObjectWrapper;
    
    // Python methods for the wrapper
    static PyObject* wrapper_getattr(JSObjectWrapper* self, PyObject* attr_name);
    static int wrapper_setattr(JSObjectWrapper* self, PyObject* attr_name, PyObject* value);
    static PyObject* wrapper_call(JSObjectWrapper* self, PyObject* args, PyObject* kwargs);
};

}