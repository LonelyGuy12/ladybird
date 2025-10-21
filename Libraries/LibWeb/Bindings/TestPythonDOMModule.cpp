/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/TestPythonDOMModule.h>
#include <LibWeb/DOM/Document.h>
#include <LibWeb/DOM/Element.h>
#include <LibWeb/HTML/Window.h>
#include <LibWeb/Bindings/PythonDOMBindings.h>
#include <LibCore/File.h>
#include <LibWeb/HTML/HTMLParser.h>

namespace Web::Bindings {

PyObject* TestPythonDOMModule::s_module = nullptr;
bool TestPythonDOMModule::s_initialized = false;

// Test functions
static PyObject* test_document_access(PyObject* self, PyObject* args)
{
    // Create a mock document
    auto document = DOM::Document::create();
    auto parser = HTML::HTMLParser::create(document, "<html><body></body></html>", "utf-8");
    parser->run();

    // Create a Python wrapper for the document
    PyObject* doc_wrapper = PythonDocument::create_from_cpp_document(*document);
    if (!doc_wrapper) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create document wrapper");
        return nullptr;
    }

    // Execute the Python test script
    auto file = Core::File::open("opencode.py", Core::File::OpenMode::Read);
    if (file.is_error()) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to open opencode.py");
        return nullptr;
    }
    auto buffer = file.value()->read_until_eof();

    PyObject* globals = PyDict_New();
    PyObject* locals = PyDict_New();
    PyDict_SetItemString(globals, "web", PythonDOMAPI::get_module());
    PyDict_SetItemString(globals, "document", doc_wrapper);

    PyObject* result = PyRun_String(reinterpret_cast<const char*>(buffer.data()), Py_file_input, globals, locals);
    if (!result) {
        PyErr_Print();
        PyErr_SetString(PyExc_RuntimeError, "Python test script failed to execute");
        return nullptr;
    }

    Py_DECREF(doc_wrapper);
    Py_DECREF(globals);
    Py_DECREF(locals);

    Py_RETURN_TRUE;
}

static PyObject* test_element_manipulation(PyObject* self, PyObject* args)
{
    // This is a placeholder for testing element manipulation
    Py_RETURN_TRUE;
}

static PyObject* test_window_object(PyObject* self, PyObject* args)
{
    // This is a placeholder for testing window object access
    Py_RETURN_TRUE;
}

// Module methods
static PyMethodDef s_module_methods[] = {
    {"test_document_access", test_document_access, METH_NOARGS, "Test document access"},
    {"test_element_manipulation", test_element_manipulation, METH_NOARGS, "Test element manipulation"},
    {"test_window_object", test_window_object, METH_NOARGS, "Test window object access"},
    {NULL, NULL, 0, NULL}  // Sentinel
};

static struct PyModuleDef test_module_def = {
    PyModuleDef_HEAD_INIT,
    .m_name = "test_web",
    .m_doc = "Test Python Web API module",
    .m_size = -1,
    .m_methods = s_module_methods,
};

bool TestPythonDOMModule::initialize_module()
{
    if (s_initialized) {
        return true;
    }
    
    // Create the test module
    s_module = PyModule_Create(&test_module_def);
    if (!s_module) {
        return false;
    }
    
    s_initialized = true;
    return true;
}

PyObject* TestPythonDOMModule::get_module()
{
    if (!s_initialized) {
        return nullptr;
    }
    return s_module;
}

}