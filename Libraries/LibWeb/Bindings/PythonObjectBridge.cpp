/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/PythonObjectBridge.h>
#include <LibWeb/DOM/Document.h>
#include <LibWeb/DOM/Element.h>
#include <LibWeb/HTML/Window.h>
#include <LibWeb/HTML/Location.h>
#include <LibWeb/HTML/History.h>

namespace Web::Bindings {

// Type registry to keep track of the C++ type of wrapped objects
static PyObject* s_cpp_object_map = nullptr;

PyObject* PythonObjectBridge::create_wrapper_for_dom_object(void* cpp_object, const char* type_name)
{
    if (!cpp_object || !type_name) {
        Py_RETURN_NONE;
    }
    
    // Create a Python capsule to hold the C++ object pointer
    // A capsule is a Python object that holds a C pointer
    PyObject* capsule = PyCapsule_New(cpp_object, type_name, nullptr);
    if (!capsule) {
        return nullptr;
    }
    
    // Store the capsule in our registry with the object pointer as key
    // This allows us to retrieve the object later
    if (!s_cpp_object_map) {
        s_cpp_object_map = PyDict_New();
        if (!s_cpp_object_map) {
            Py_DECREF(capsule);
            return nullptr;
        }
    }
    
    // Create a string representation of the pointer address as key
    char key[32];
    snprintf(key, sizeof(key), "%p", cpp_object);
    
    if (PyDict_SetItemString(s_cpp_object_map, key, capsule) < 0) {
        Py_DECREF(capsule);
        return nullptr;
    }
    
    return capsule;
}

void* PythonObjectBridge::get_cpp_object_from_wrapper(PyObject* wrapper)
{
    if (!wrapper || !PyCapsule_CheckExact(wrapper)) {
        return nullptr;
    }
    
    void* cpp_object = PyCapsule_GetPointer(wrapper, nullptr);
    return cpp_object;
}

bool PythonObjectBridge::is_dom_wrapper(PyObject* obj)
{
    if (!obj) {
        return false;
    }
    
    return PyCapsule_CheckExact(obj);
}

void PythonObjectBridge::initialize_bindings()
{
    // Initialize the object mapping dictionary
    s_cpp_object_map = PyDict_New();
}

// Python method to get document
static PyObject* python_window_document(PyObject* self, PyObject* args)
{
    void* cpp_obj = PythonObjectBridge::get_cpp_object_from_wrapper(self);
    if (!cpp_obj) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid window object");
        return nullptr;
    }
    
    auto* window = static_cast<HTML::Window*>(cpp_obj);
    auto& document = window->document();
    
    // Create a Python wrapper for the document
    PyObject* doc_wrapper = PythonObjectBridge::create_wrapper_for_dom_object(&document, "Document");
    
    Py_XINCREF(doc_wrapper);
    return doc_wrapper;
}

// Python method to get location
static PyObject* python_window_location(PyObject* self, PyObject* args)
{
    void* cpp_obj = PythonObjectBridge::get_cpp_object_from_wrapper(self);
    if (!cpp_obj) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid window object");
        return nullptr;
    }
    
    auto* window = static_cast<HTML::Window*>(cpp_obj);
    auto& location = window->location();
    
    // Create a Python wrapper for the location
    PyObject* location_wrapper = PythonObjectBridge::create_wrapper_for_dom_object(&location, "Location");
    
    Py_XINCREF(location_wrapper);
    return location_wrapper;
}

// Python method to get history
static PyObject* python_window_history(PyObject* self, PyObject* args)
{
    void* cpp_obj = PythonObjectBridge::get_cpp_object_from_wrapper(self);
    if (!cpp_obj) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid window object");
        return nullptr;
    }
    
    auto* window = static_cast<HTML::Window*>(cpp_obj);
    auto& history = window->history();
    
    // Create a Python wrapper for the history
    PyObject* history_wrapper = PythonObjectBridge::create_wrapper_for_dom_object(&history, "History");
    
    Py_XINCREF(history_wrapper);
    return history_wrapper;
}

// Python method to write to console
static PyObject* python_console_log(PyObject* self, PyObject* args)
{
    const char* message = nullptr;
    if (!PyArg_ParseTuple(args, "s", &message)) {
        return nullptr;
    }
    
    // Instead of using Python's print, we'll use Ladybird's logging
    dbgln("Python console.log: {}", message);
    
    Py_RETURN_NONE;
}

// Method definitions for Window object
static PyMethodDef window_methods[] = {
    {"document", (PyCFunction)python_window_document, METH_NOARGS, "Get document object"},
    {"location", (PyCFunction)python_window_location, METH_NOARGS, "Get location object"},
    {"history", (PyCFunction)python_window_history, METH_NOARGS, "Get history object"},
    {NULL}  // Sentinel
};

// Method definitions for Console object
static PyMethodDef console_methods[] = {
    {"log", (PyCFunction)python_console_log, METH_VARARGS, "Log a message to console"},
    {NULL}  // Sentinel
};

bool PythonObjectBridge::add_dom_apis_to_module(PyObject* module)
{
    if (!module) {
        return false;
    }
    
    // Add console object
    PyObject* console_obj = PyDict_New();
    if (!console_obj) {
        return false;
    }
    
    // Add console methods to the console object
    for (int i = 0; console_methods[i].ml_name != NULL; i++) {
        PyMethodDef method = console_methods[i];
        if (method.ml_name) {
            PyObject* method_obj = PyCFunction_New(&method, NULL);
            if (!method_obj) {
                Py_DECREF(console_obj);
                return false;
            }
            PyDict_SetItemString(console_obj, method.ml_name, method_obj);
            Py_DECREF(method_obj);
        }
    }
    
    PyModule_AddObject(module, "console", console_obj);
    
    return true;
}

}