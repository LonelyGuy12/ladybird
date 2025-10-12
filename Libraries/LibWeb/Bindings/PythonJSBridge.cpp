/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/PythonJSBridge.h>
#include <LibWeb/Bindings/PythonJSObjectWrapper.h>
#include <LibJS/Runtime/Value.h>
#include <LibJS/Runtime/PrimitiveString.h>
#include <LibJS/Runtime/Object.h>
#include <LibJS/Runtime/Array.h>
#include <LibJS/Runtime/FunctionObject.h>
#include <LibJS/Heap/Heap.h>

namespace Web::Bindings {

bool PythonJSBridge::s_bridge_initialized = false;
PyObject* PythonJSBridge::s_js_proxy_type = nullptr;
PyObject* PythonJSBridge::s_py_proxy_type = nullptr;

bool PythonJSBridge::initialize_bridge()
{
    if (s_bridge_initialized) {
        return true;
    }
    
    s_bridge_initialized = true;
    return true;
}

JS::Value PythonJSBridge::python_to_js(PyObject* py_obj, JS::Realm& realm)
{
    if (!py_obj) {
        return JS::js_undefined();
    }
    
    if (py_obj == Py_None) {
        return JS::js_undefined();
    }
    
    if (PyBool_Check(py_obj)) {
        return JS::Value(py_obj == Py_True);
    }
    
    if (PyLong_Check(py_obj)) {
        auto value = PyLong_AsLong(py_obj);
        if (PyErr_Occurred()) {
            PyErr_Clear();
            // Handle large integers
            double double_val = PyLong_AsDouble(py_obj);
            if (!PyErr_Occurred()) {
                return JS::Value(double_val);
            }
            return JS::js_undefined();
        }
        return JS::Value(static_cast<i32>(value));
    }
    
    if (PyFloat_Check(py_obj)) {
        return JS::Value(PyFloat_AsDouble(py_obj));
    }
    
    if (PyUnicode_Check(py_obj)) {
        const char* str = PyUnicode_AsUTF8(py_obj);
        if (str) {
            return JS::PrimitiveString::create(realm.vm(), String::from_utf8(str).release_value_but_fixme_should_propagate_errors());
        }
        return JS::js_undefined();
    }
    
    if (PyDict_Check(py_obj)) {
        // Create a JS object from Python dict
        auto js_obj = JS::Object::create(realm, realm.intrinsics().object_prototype());
        
        PyObject *key, *value;
        Py_ssize_t pos = 0;
        
        while (PyDict_Next(py_obj, &pos, &key, &value)) {
            if (PyUnicode_Check(key)) {
                const char* key_str = PyUnicode_AsUTF8(key);
                if (key_str) {
                    auto js_value = python_to_js(value, realm);
                    js_obj->put_without_side_effects(key_str, js_value);
                }
            }
        }
        
        return js_obj;
    }
    
    if (PyList_Check(py_obj) || PyTuple_Check(py_obj)) {
        // Create a JS array from Python list/tuple
        auto js_array = JS::Array::create(realm, 0);
        Py_ssize_t size = PySequence_Size(py_obj);
        
        for (Py_ssize_t i = 0; i < size; ++i) {
            PyObject* item = PySequence_GetItem(py_obj, i);
            if (item) {
                auto js_value = python_to_js(item, realm);
                js_array->set_index(i, js_value);
                Py_DECREF(item);
            }
        }
        
        return js_array;
    }
    
    // For other Python objects, create a proxy wrapper
    // This would be a more complex implementation in a real system
    return JS::js_undefined();
}

PyObject* PythonJSBridge::js_to_python(JS::Value js_val)
{
    auto& vm = js_val.as_pointer()->shape().realm().vm();
    
    if (js_val.is_undefined() || js_val.is_null()) {
        Py_RETURN_NONE;
    }
    
    if (js_val.is_boolean()) {
        return js_val.as_bool() ? Py_True : Py_False;
    }
    
    if (js_val.is_number()) {
        double num = js_val.as_double();
        if (num == (i32)num) {
            return PyLong_FromLong((i32)num);
        }
        return PyFloat_FromDouble(num);
    }
    
    if (js_val.is_string()) {
        auto str = js_val.as_string().string();
        return PyUnicode_FromString(str.to_utf8().characters());
    }
    
    if (js_val.is_object()) {
        auto& obj = js_val.as_object();
        
        // Check if it's an array
        if (is<JS::Array>(obj)) {
            auto& js_array = static_cast<JS::Array&>(obj);
            PyObject* py_list = PyList_New(0);
            
            for (size_t i = 0; i < js_array.array_like_size(); ++i) {
                auto element = js_array.get_index(i);
                if (!element.is_exception()) {
                    PyObject* py_elem = js_to_python(element.release_value());
                    if (py_elem) {
                        PyList_Append(py_list, py_elem);
                        Py_DECREF(py_elem);
                    }
                }
            }
            
            return py_list;
        }
        
        // For other objects, we'd create a more sophisticated proxy
        // For now, return None as a placeholder
        Py_RETURN_NONE;
    }
    
    // For other types, return None as a default
    Py_RETURN_NONE;
}

PyObject* PythonJSBridge::call_js_function(const String& function_name, PyObject* args, JS::Realm& realm)
{
    // This is a simplified implementation
    // In a real implementation, we would need to:
    // 1. Find the JavaScript function in the global scope
    // 2. Convert Python arguments to JS values
    // 3. Call the function
    // 4. Convert the result back to Python
    
    // For now, return None as a placeholder
    Py_RETURN_NONE;
}

JS::Value PythonJSBridge::call_python_function(PyObject* func, JS::MarkedVector<JS::Value>& js_args, JS::Realm& realm)
{
    // Convert JS arguments to Python arguments
    PyObject* py_args = PyTuple_New(js_args.size());
    if (!py_args) {
        return JS::js_undefined();
    }
    
    for (size_t i = 0; i < js_args.size(); ++i) {
        PyObject* py_arg = js_to_python(js_args[i]);
        if (!py_arg) {
            Py_DECREF(py_args);
            return JS::js_undefined();
        }
        PyTuple_SetItem(py_args, i, py_arg);  // Steals reference
    }
    
    // Call the Python function
    PyObject* result = PyObject_Call(func, py_args, nullptr);
    Py_DECREF(py_args);
    
    if (!result) {
        PyErr_Clear();
        return JS::js_undefined();
    }
    
    // Convert result back to JS value
    JS::Value js_result = python_to_js(result, realm);
    Py_DECREF(result);
    
    return js_result;
}

bool PythonJSBridge::setup_bridge_in_context(PyObject* globals, JS::Realm& realm)
{
    if (!globals) {
        return false;
    }
    
    // Add bridge functions to the global context
    // This could include functions like `js_eval`, `call_js_function`, etc.
    
    // Create a bridge object that can access JS functions
    PyObject* bridge_module = PyImport_AddModule("js_bridge");
    if (!bridge_module) {
        return false;
    }
    
    // Add functions to enable cross-language communication
    // For example, a function to access the JavaScript global object
    auto& global_obj = realm.global_object();
    PyObject* js_global_wrapper = PythonJSObjectWrapper::create_wrapper(global_obj);
    if (js_global_wrapper) {
        PyModule_AddObject(bridge_module, "window", js_global_wrapper);
        // js_global_wrapper is now owned by the module
    }
    
    // Also add it directly to globals for convenience
    if (js_global_wrapper) {
        Py_INCREF(js_global_wrapper);  // Increment for globals
        PyDict_SetItemString(globals, "window", js_global_wrapper);
    }
    
    PyModule_AddObject(globals, "js_bridge", bridge_module);
    Py_INCREF(bridge_module);  // Keep reference since PyImport_AddModule doesn't
    
    return true;
}

}