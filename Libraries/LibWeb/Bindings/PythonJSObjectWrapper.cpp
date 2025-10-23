/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/PythonJSObjectWrapper.h>
#include <LibWeb/Bindings/PythonJSBridge.h>
#include <LibJS/Runtime/ExecutionContext.h>
#include <LibJS/Runtime/FunctionObject.h>
#include <LibJS/Runtime/Object.h>
#include <LibJS/Runtime/PropertyKey.h>
#include <LibJS/Runtime/Realm.h>
#include <LibJS/Runtime/Value.h>
#include <AK/Utf16String.h>

namespace Web::Bindings {

PyTypeObject PythonJSObjectWrapper::s_js_object_wrapper_type = { PyVarObject_HEAD_INIT(NULL, 0) };

void PythonJSObjectWrapper::setup_js_object_wrapper_type()
{
    if (s_js_object_wrapper_type.tp_name) return; // Already set up
    
    memset(&s_js_object_wrapper_type, 0, sizeof(PyTypeObject));
    PyVarObject_HEAD_INIT(&s_js_object_wrapper_type, 0)
    s_js_object_wrapper_type.tp_name = "web.JSObject";
    s_js_object_wrapper_type.tp_doc = "JavaScript object wrapper for Python";
    s_js_object_wrapper_type.tp_basicsize = sizeof(JSObjectWrapper);
    s_js_object_wrapper_type.tp_itemsize = 0;
    s_js_object_wrapper_type.tp_flags = Py_TPFLAGS_DEFAULT;
    s_js_object_wrapper_type.tp_methods = NULL;
    s_js_object_wrapper_type.tp_getattro = (getattrofunc)wrapper_getattr;
    s_js_object_wrapper_type.tp_setattro = (setattrofunc)wrapper_setattr;
    s_js_object_wrapper_type.tp_call = (ternaryfunc)wrapper_call;
    
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
    auto& realm = js_obj->shape().realm();
    auto& vm = realm.vm();
    
    // Convert Python string to JS PropertyKey
    if (PyUnicode_Check(attr_name)) {
        const char* attr_str = PyUnicode_AsUTF8(attr_name);
        if (!attr_str) {
            return nullptr;
        }
        
        auto attr_view = StringView { attr_str, strlen(attr_str) };
        auto attr_utf16 = Utf16String::from_utf8(attr_view);
        auto js_key = JS::PropertyKey(attr_utf16);
        auto js_value = js_obj->get(js_key);
        
        if (js_value.is_error()) {
            PyErr_SetString(PyExc_AttributeError, attr_str);
            return nullptr;
        }
        
        return PythonJSBridge::js_to_python(js_value.release_value(), vm);
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
    auto& realm = js_obj->shape().realm();
    auto& vm = realm.vm();
    
    // Convert Python string to JS PropertyKey and Python value to JS value
    if (PyUnicode_Check(attr_name)) {
        const char* attr_str = PyUnicode_AsUTF8(attr_name);
        if (!attr_str) {
            return -1;
        }
        
        auto attr_view = StringView { attr_str, strlen(attr_str) };
        auto attr_utf16 = Utf16String::from_utf8(attr_view);
        auto js_key = JS::PropertyKey(attr_utf16);
        auto js_value = PythonJSBridge::python_to_js(value, realm);
        
        auto result = js_obj->set(js_key, js_value, JS::Object::ShouldThrowExceptions::Yes);
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
    
    (void)kwargs; // Unused parameter
    
    JS::Object* js_obj = static_cast<JS::Object*>(self->js_object_ptr);
    auto& realm = js_obj->shape().realm();
    auto& vm = realm.vm();
    
    if (!is<JS::FunctionObject>(*js_obj)) {
        PyErr_SetString(PyExc_TypeError, "JavaScript object is not callable");
        return nullptr;
    }
    
    auto& js_func = static_cast<JS::FunctionObject&>(*js_obj);
    
    // Convert Python arguments to JS values
    Py_ssize_t arg_count = PyTuple_Size(args);
    auto js_args = GC::RootVector<JS::Value>(vm.heap());
    js_args.ensure_capacity(arg_count);
    
    for (Py_ssize_t i = 0; i < arg_count; ++i) {
        PyObject* py_arg = PyTuple_GetItem(args, i);
        if (py_arg) {
            auto js_arg = PythonJSBridge::python_to_js(py_arg, realm);
            js_args.unchecked_append(js_arg);
        }
    }
    
    // Call the JS function using internal_call
    auto execution_context = JS::ExecutionContext::create();
    execution_context->arguments = js_args.span();
    auto result = js_func.internal_call(*execution_context, JS::js_undefined());
    
    if (result.is_error()) {
        PyErr_SetString(PyExc_RuntimeError, "Error calling JavaScript function");
        return nullptr;
    }
    
    // Convert result back to Python
    return PythonJSBridge::js_to_python(result.release_value(), vm);
}

}