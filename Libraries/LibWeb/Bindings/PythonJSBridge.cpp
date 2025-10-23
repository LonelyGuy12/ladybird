/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/StringView.h>
#include <AK/Utf16String.h>
#include <LibWeb/Bindings/PythonJSBridge.h>
#include <LibWeb/Bindings/PythonCompat.h>
#include <LibJS/Runtime/Array.h>
#include <LibJS/Runtime/ExecutionContext.h>
#include <LibJS/Runtime/FunctionObject.h>
#include <LibJS/Runtime/Object.h>
#include <LibJS/Runtime/PropertyKey.h>
#include <LibJS/Runtime/Realm.h>
#include <LibJS/Runtime/Value.h>
#include <AK/StringView.h>
#include <AK/Utf16String.h>
#include <LibWeb/Bindings/PythonJSObjectWrapper.h>
#include <cstring>

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
        char const* str = PyUnicode_AsUTF8(py_obj);
        if (str) {
            return JS::PrimitiveString::create(realm.vm(), MUST(String::from_utf8(StringView { str, strlen(str) })));
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
                char const* key_str = PyUnicode_AsUTF8(key);
                if (key_str) {
                    auto js_value = python_to_js(value, realm);
                    auto key_view = StringView { key_str, strlen(key_str) };
                    auto key_utf16 = Utf16String::from_utf8(key_view);
                    auto property_key = JS::PropertyKey(key_utf16);
                    MUST(js_obj->create_data_property(property_key, js_value));
                }
            }
        }

        return js_obj;
    }

    if (PyList_Check(py_obj) || PyTuple_Check(py_obj)) {
        // Create a JS array from Python list/tuple
        auto js_array = MUST(JS::Array::create(realm, 0));
        Py_ssize_t size = PySequence_Size(py_obj);

        for (Py_ssize_t i = 0; i < size; ++i) {
            PyObject* item = PySequence_GetItem(py_obj, i);
            if (item) {
                auto js_value = python_to_js(item, realm);
                MUST(js_array->set(JS::PropertyKey(static_cast<u32>(i)), js_value, JS::Object::ShouldThrowExceptions::Yes));
                Py_DECREF(item);
            }
        }

        return js_array;
    }

    // For other Python objects, create a proxy wrapper
    // This would be a more complex implementation in a real system
    return JS::js_undefined();
}

PyObject* PythonJSBridge::js_to_python(JS::Value js_val, JS::VM& vm)
{
    if (js_val.is_undefined() || js_val.is_null()) {
        Py_RETURN_NONE;
    }

    if (js_val.is_boolean()) {
        return js_val.as_bool() ? Py_True : Py_False;
    }

    if (js_val.is_number()) {
        return PyFloat_FromDouble(js_val.as_double());
    }

    if (js_val.is_string()) {
        auto str = js_val.as_string().utf8_string();
        auto byte_str = str.to_byte_string();
        return PyUnicode_FromString(byte_str.characters());
    }

    if (js_val.is_object()) {
        auto& obj = js_val.as_object();

        // Handle arrays
        if (is<JS::Array>(obj)) {
            auto& array = static_cast<JS::Array&>(obj);
            Py_ssize_t length = static_cast<Py_ssize_t>(array.indexed_properties().array_like_size());

            PyObject* py_list = PyList_New(length);
            if (!py_list) {
                return nullptr;
            }

            for (Py_ssize_t i = 0; i < length; ++i) {
                auto element = MUST(array.get(JS::PropertyKey(static_cast<u32>(i))));
                PyObject* py_element = js_to_python(element, vm);
                if (!py_element) {
                    Py_DECREF(py_list);
                    return nullptr;
                }
                PyList_SetItem(py_list, i, py_element);
            }

            return py_list;
        }

        // Handle regular objects
        PyObject* py_dict = PyDict_New();
        if (!py_dict) {
            return nullptr;
        }

        // Get all enumerable properties
        auto properties = MUST(obj.internal_own_property_keys());
        for (auto& property_value : properties) {
            if (!property_value.is_string())
                continue;
            auto key_str = property_value.as_string().utf8_string();
            auto property_key_result = JS::PropertyKey::from_value(vm, property_value);
            if (property_key_result.is_error()) {
                Py_DECREF(py_dict);
                return nullptr;
            }
            auto property_key = property_key_result.release_value();
            auto value = MUST(obj.get(property_key));
            auto key_byte_str = key_str.to_byte_string();
            PyObject* py_key = PyUnicode_FromString(key_byte_str.characters());
            PyObject* py_value = js_to_python(value, vm);

            if (!py_key || !py_value) {
                Py_DECREF(py_key);
                Py_DECREF(py_value);
                Py_DECREF(py_dict);
                return nullptr;
            }

            PyDict_SetItem(py_dict, py_key, py_value);
            Py_DECREF(py_key);
            Py_DECREF(py_value);
        }

        return py_dict;
    }

    // For other types, return None
    Py_RETURN_NONE;
}

PyObject* PythonJSBridge::call_js_function(String const& function_name, PyObject* args, JS::Realm& realm)
{
    auto& vm = realm.vm();

    // Lookup function on JS global object by name
    auto& global_obj = realm.global_object();
    auto function_name_view = function_name.bytes_as_string_view();
    auto utf16_name = Utf16String::from_utf8(function_name_view);
    auto property_key = JS::PropertyKey(utf16_name);
    auto get_result = global_obj.get(property_key);
    if (get_result.is_error()) {
        Py_RETURN_NONE;
    }
    auto js_value = get_result.release_value();
    if (!js_value.is_function()) {
        Py_RETURN_NONE;
    }

    auto& js_function = js_value.as_function();

    // Convert Python tuple args to JS arguments
    Py_ssize_t argc = args ? PyTuple_Size(args) : 0;
    auto js_args = GC::RootVector<JS::Value>(vm.heap());
    js_args.ensure_capacity(static_cast<size_t>(max<Py_ssize_t>(0, argc)));
    for (Py_ssize_t i = 0; i < argc; ++i) {
        PyObject* py_arg = PyTuple_GetItem(args, i); // Borrowed ref
        auto converted = python_to_js(py_arg, realm);
        js_args.unchecked_append(converted);
    }

    // Call the JS function using internal_call
    auto execution_context = JS::ExecutionContext::create(0, static_cast<u32>(argc));
    execution_context->arguments = js_args.span();
    auto call_result = js_function.internal_call(*execution_context, JS::js_undefined());
    if (call_result.is_error()) {
        Py_RETURN_NONE;
    }
    return js_to_python(call_result.release_value(), vm);
}

JS::Value PythonJSBridge::call_python_function(PyObject* func, GC::RootVector<JS::Value>& js_args, JS::Realm& realm)
{
    // Convert JS arguments to Python arguments
    PyObject* py_args = PyTuple_New(js_args.size());
    if (!py_args) {
        return JS::js_undefined();
    }

    for (size_t i = 0; i < js_args.size(); ++i) {
        PyObject* py_arg = js_to_python(js_args[i], realm.vm());
        if (!py_arg) {
            Py_DECREF(py_args);
            return JS::js_undefined();
        }
        PyTuple_SetItem(py_args, i, py_arg); // Steals reference
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
        Py_INCREF(js_global_wrapper); // Increment for globals
        PyDict_SetItemString(globals, "window", js_global_wrapper);
    }

    // Register bridge module in globals dict
    // PyImport_AddModule returns a borrowed reference; PyDict_SetItemString will INCREF it.
    if (PyDict_SetItemString(globals, "js_bridge", bridge_module) < 0) {
        return false;
    }

    return true;
}

}
