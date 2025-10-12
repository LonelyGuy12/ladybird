/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/PythonJSObjectWrapper.h>
#include <LibWeb/Bindings/PythonJSBridge.h>
#include <LibJS/Runtime/Object.h>
#include <LibJS/Runtime/PropertyName.h>
#include <LibJS/Runtime/Value.h>
#include <LibJS/PropertyKey.h>

namespace Web::Bindings {

PyTypeObject PythonJSObjectWrapper::s_js_object_wrapper_type = {0};

void PythonJSObjectWrapper::setup_js_object_wrapper_type()
{
    if (s_js_object_wrapper_type.tp_name) return; // Already set up
    
    static PyTypeObject type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "web.JSObject",
        .tp_doc = "JavaScript object wrapper for Python",
        .tp_basicsize = sizeof(JSObjectWrapper),
        .tp_itemsize = 0,
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_methods = NULL,
        .tp_getattro = (getattrofunc)wrapper_getattr,
        .tp_setattro = (setattrofunc)wrapper_setattr,
        .tp_call = (ternaryfunc)wrapper_call,
    };
    
    memcpy(&s_js_object_wrapper_type, &type, sizeof(PyTypeObject));
    
    if (PyType_Ready(&s_js_object_wrapper_type) < 0) {
        // Handle error appropriately
    }
}

PyObject* PythonJSObjectWrapper::create_wrapper(JS::Object& js_obj)
{
    setup_js_object_wrapper_type();
    
    JSObjectWrapper* wrapper = PyObject_New(JSObjectWrapper, &s_js_object_wrapper_type);
    if (wrapper) {
        wrapper->js_object_ptr = &js_obj;
    }
    return (PyObject*)wrapper;
}

JS::Object* PythonJSObjectWrapper::get_js_object(PyObject* wrapper)
{
    if (!wrapper || !PyObject_TypeCheck(wrapper, &s_js_object_wrapper_type)) {
        return nullptr;
    }
    
    JSObjectWrapper* js_wrapper = (JSObjectWrapper*)wrapper;
    return static_cast<JS::Object*>(js_wrapper->js_object_ptr);
}

PyObject* PythonJSObjectWrapper::wrapper_getattr(JSObjectWrapper* self, PyObject* attr_name)
{
    if (!self->js_object_ptr) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid JavaScript object wrapper");
        return nullptr;
    }
    
    JS::Object* js_obj = static_cast<JS::Object*>(self->js_object_ptr);
    auto& vm = js_obj->shape().realm().vm();
    
    // Convert Python string to JS PropertyKey
    if (PyUnicode_Check(attr_name)) {
        const char* attr_str = PyUnicode_AsUTF8(attr_name);
        if (!attr_str) {
            return nullptr;
        }
        
        auto js_key = JS::PropertyKey::symbol_or_string(vm, attr_str);
        auto js_value = js_obj->get(js_key);
        
        if (js_value.is_error()) {
            PyErr_SetString(PyExc_AttributeError, attr_str);
            return nullptr;
        }
        
        return PythonJSBridge::js_to_python(js_value.release_value());
    }
    
    PyErr_SetString(PyExc_TypeError, "Attribute name must be a string");
    return nullptr;
}

int PythonJSObjectWrapper::wrapper_setattr(JSObjectWrapper* self, PyObject* attr_name, PyObject* value)
{
    if (!self->js_object_ptr) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid JavaScript object wrapper");
        return -1;
    }
    
    JS::Object* js_obj = static_cast<JS::Object*>(self->js_object_ptr);
    auto& vm = js_obj->shape().realm().vm();
    
    // Convert Python string to JS PropertyKey and Python value to JS value
    if (PyUnicode_Check(attr_name)) {
        const char* attr_str = PyUnicode_AsUTF8(attr_name);
        if (!attr_str) {
            return -1;
        }
        
        auto js_key = JS::PropertyKey::symbol_or_string(vm, attr_str);
        auto js_value = PythonJSBridge::python_to_js(value, js_obj->shape().realm());
        
        auto result = js_obj->put(js_key, js_value);
        if (result.is_error()) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to set property");
            return -1;
        }
        
        return 0;
    }
    
    PyErr_SetString(PyExc_TypeError, "Attribute name must be a string");
    return -1;
}

PyObject* PythonJSObjectWrapper::wrapper_call(JSObjectWrapper* self, PyObject* args, PyObject* kwargs)
{
    if (!self->js_object_ptr) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid JavaScript object wrapper");
        return nullptr;
    }
    
    JS::Object* js_obj = static_cast<JS::Object*>(self->js_object_ptr);
    auto& realm = js_obj->shape().realm();
    auto& vm = realm.vm();
    
    if (!js_obj->is_function()) {
        PyErr_SetString(PyExc_TypeError, "JavaScript object is not callable");
        return nullptr;
    }
    
    JS::FunctionObject& js_func = static_cast<JS::FunctionObject&>(*js_obj);
    
    // Convert Python arguments to JS values
    Py_ssize_t arg_count = PyTuple_Size(args);
    JS::MarkedVector<JS::Value> js_args(vm.heap());
    js_args.ensure_capacity(arg_count);
    
    for (Py_ssize_t i = 0; i < arg_count; ++i) {
        PyObject* py_arg = PyTuple_GetItem(args, i);
        if (py_arg) {
            auto js_arg = PythonJSBridge::python_to_js(py_arg, realm);
            js_args.unchecked_append(js_arg);
        }
    }
    
    // Call the JS function
    auto result = JS::call(vm, js_func, JS::js_undefined(), move(js_args));
    
    if (result.is_error()) {
        PyErr_SetString(PyExc_RuntimeError, "Error calling JavaScript function");
        return nullptr;
    }
    
    // Convert result back to Python
    return PythonJSBridge::js_to_python(result.release_value());
}

}