/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/PythonDOMBindings.h>
#include <LibWeb/Bindings/PythonDOMWrapperCache.h>
#include <LibWeb/Bindings/TestPythonDOMModule.h>
#include <LibWeb/DOM/Document.h>
#include <LibWeb/DOM/Element.h>
#include <LibWeb/DOM/HTMLCollection.h>
#include <LibWeb/DOM/NodeList.h>
#include <LibWeb/HTML/History.h>
#include <LibWeb/HTML/Location.h>
#include <LibWeb/HTML/Window.h>
#include <LibGC/Heap.h>

namespace Web::Bindings {

// Python object structure for Document
typedef struct {
    PyObject_HEAD
        Web::DOM::Document* document;
} PythonDocumentObject;

typedef struct {
    PyObject_HEAD
        Web::DOM::Element* element;
} PythonElementObject;

typedef struct {
    PyObject_HEAD
        Web::HTML::Window* window;
} PythonWindowObject;

// Forward declarations for type objects
PyTypeObject PythonDocument::s_type;
PyTypeObject PythonElement::s_type;
PyTypeObject PythonWindow::s_type;

PyObject* PythonDOMAPI::s_module = nullptr;
bool PythonDOMAPI::s_initialized = false;



// PythonDocument methods
static void python_document_dealloc(PythonDocumentObject* self)
{
    // Don't delete the C++ document, just the Python wrapper
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* python_document_select(PythonDocumentObject* self, PyObject* args)
{
    if (!self->document) {
        PyErr_SetString(PyExc_RuntimeError, "Document object is invalid");
        return nullptr;
    }

    char const* selector = nullptr;
    if (!PyArg_ParseTuple(args, "s", &selector)) {
        return nullptr;
    }

    // Use the document's querySelectorAll method (converted to Python API)
    auto selector_view = StringView(selector);
    auto elements = self->document->query_selector_all(selector_view);
    if (elements.is_error()) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid selector");
        return nullptr;
    }

    // Create a Python list to hold the results
    PyObject* result = PyList_New(0);
    if (!result)
        return nullptr;

    auto node_list = elements.release_value();
    for (size_t i = 0; i < node_list->length(); ++i) {
        auto node = node_list->item(i);
        if (node && is<Web::DOM::Element>(*node)) {
            auto& element = static_cast<Web::DOM::Element&>(*node);
            PyObject* element_wrapper = PythonElement::create_from_cpp_element(element);
            if (element_wrapper) {
                PyList_Append(result, element_wrapper);
                Py_DECREF(element_wrapper);
            }
        }
    }

    return result;
}

static PyObject* python_document_find(PythonDocumentObject* self, PyObject* args)
{
    if (!self->document) {
        PyErr_SetString(PyExc_RuntimeError, "Document object is invalid");
        return nullptr;
    }

    char const* selector = nullptr;
    if (!PyArg_ParseTuple(args, "s", &selector)) {
        return nullptr;
    }

    // Use the document's querySelector method (converted to Python API)
    auto selector_view = StringView(selector);
    auto element = self->document->query_selector(selector_view);
    if (element.is_error()) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid selector");
        return nullptr;
    }

    auto found_element = element.release_value();
    if (found_element) {
        return PythonElement::create_from_cpp_element(*found_element);
    }

    Py_RETURN_NONE;
}

static PyObject* python_document_create_element(PythonDocumentObject* self, PyObject* args)
{
    if (!self->document) {
        PyErr_SetString(PyExc_RuntimeError, "Document object is invalid");
        return nullptr;
    }

    char const* tag_name = nullptr;
    if (!PyArg_ParseTuple(args, "s", &tag_name)) {
        return nullptr;
    }

    auto tag_name_str = String(tag_name);
    auto element = self->document->create_element(tag_name_str, {});
    if (element.is_error()) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid element tag name");
        return nullptr;
    }

    return PythonElement::create_from_cpp_element(*element.release_value());
}

static PyMethodDef python_document_methods[] = {
    { "select", (PyCFunction)python_document_select, METH_VARARGS, "Select elements using CSS selector" },
    { "find", (PyCFunction)python_document_find, METH_VARARGS, "Find first element using CSS selector" },
    { "create_element", (PyCFunction)python_document_create_element, METH_VARARGS, "Create a new element" },
    { NULL } // Sentinel
};

static PyTypeObject document_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "web.Document",              /* tp_name */
    sizeof(PythonDocumentObject), /* tp_basicsize */
    0,                           /* tp_itemsize */
    (destructor)python_document_dealloc, /* tp_dealloc */
    0,                           /* tp_vectorcall_offset */
    0,                           /* tp_getattr */
    0,                           /* tp_setattr */
    0,                           /* tp_as_async */
    0,                           /* tp_repr */
    0,                           /* tp_as_number */
    0,                           /* tp_as_sequence */
    0,                           /* tp_as_mapping */
    0,                           /* tp_hash */
    0,                           /* tp_call */
    0,                           /* tp_str */
    0,                           /* tp_getattro */
    0,                           /* tp_setattro */
    0,                           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,          /* tp_flags */
    "Web Document object",       /* tp_doc */
    0,                           /* tp_traverse */
    0,                           /* tp_clear */
    0,                           /* tp_richcompare */
    0,                           /* tp_weaklistoffset */
    0,                           /* tp_iter */
    0,                           /* tp_iternext */
    python_document_methods,     /* tp_methods */
};

void PythonDocument::setup_type()
{
    if (s_type.tp_name)
        return; // Already set up

    memcpy(&s_type, &document_type, sizeof(PyTypeObject));
    if (PyType_Ready(&s_type) < 0) {
        // Handle error appropriately
    }
}

PyObject* PythonDocument::create_from_cpp_document(Web::DOM::Document& document)
{
    setup_type();

    auto* cache = document.wrapper_cache();
    if (!cache)
        document.set_wrapper_cache(make<PythonDOMWrapperCache>());

    if (auto* wrapper = document.wrapper_cache()->get_wrapper(&document))
        return wrapper;

    PythonDocumentObject* obj = PyObject_New(PythonDocumentObject, &s_type);
    if (obj) {
        obj->document = &document;
        document.wrapper_cache()->set_wrapper(&document, (PyObject*)obj);
    }
    return (PyObject*)obj;
}

Web::DOM::Document* PythonDocument::get_cpp_document(PyObject* obj)
{
    if (!obj || !PyObject_TypeCheck(obj, &s_type)) {
        return nullptr;
    }
    return ((PythonDocumentObject*)obj)->document;
}

// PythonElement methods
static void python_element_dealloc(PythonElementObject* self)
{
    // Don't delete the C++ element, just the Python wrapper
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* python_element_select(PythonElementObject* self, PyObject* args)
{
    if (!self->element) {
        PyErr_SetString(PyExc_RuntimeError, "Element object is invalid");
        return nullptr;
    }

    char const* selector = nullptr;
    if (!PyArg_ParseTuple(args, "s", &selector)) {
        return nullptr;
    }

    // Use the element's querySelectorAll method on itself
    auto selector_view = StringView(selector);
    auto elements = self->element->query_selector_all(selector_view);
    if (elements.is_error()) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid selector");
        return nullptr;
    }

    // Create a Python list to hold the results
    PyObject* result = PyList_New(0);
    if (!result)
        return nullptr;

    auto node_list = elements.release_value();
    for (size_t i = 0; i < node_list->length(); ++i) {
        auto node = node_list->item(i);
        if (node && is<Web::DOM::Element>(*node)) {
            auto& element = static_cast<Web::DOM::Element&>(*node);
            PyObject* element_wrapper = PythonElement::create_from_cpp_element(element);
            if (element_wrapper) {
                PyList_Append(result, element_wrapper);
                Py_DECREF(element_wrapper);
            }
        }
    }

    return result;
}

static PyObject* python_element_find(PythonElementObject* self, PyObject* args)
{
    if (!self->element) {
        PyErr_SetString(PyExc_RuntimeError, "Element object is invalid");
        return nullptr;
    }

    char const* selector = nullptr;
    if (!PyArg_ParseTuple(args, "s", &selector)) {
        return nullptr;
    }

    auto selector_view = StringView(selector);
    auto element = self->element->query_selector(selector_view);
    if (element.is_error()) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid selector");
        return nullptr;
    }

    auto found_element = element.release_value();
    if (found_element) {
        return PythonElement::create_from_cpp_element(*found_element);
    }

    Py_RETURN_NONE;
}

static PyObject* python_element_get_attribute(PythonElementObject* self, PyObject* args)
{
    if (!self->element) {
        PyErr_SetString(PyExc_RuntimeError, "Element object is invalid");
        return nullptr;
    }

    char const* name = nullptr;
    if (!PyArg_ParseTuple(args, "s", &name)) {
        return nullptr;
    }

    auto name_fly = FlyString(name);
    auto value = self->element->get_attribute(name_fly);
    if (value.has_value()) {
        return PyUnicode_FromString(value->characters());
    }

    Py_RETURN_NONE;
}

static PyObject* python_element_set_attribute(PythonElementObject* self, PyObject* args)
{
    if (!self->element) {
        PyErr_SetString(PyExc_RuntimeError, "Element object is invalid");
        return nullptr;
    }

    char const* name = nullptr;
    char const* value = nullptr;
    if (!PyArg_ParseTuple(args, "ss", &name, &value)) {
        return nullptr;
    }

    auto name_fly = FlyString(name);
    auto value_str = String(value);
    auto result = self->element->set_attribute(name_fly, value_str);
    if (result.is_error()) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to set attribute");
        return nullptr;
    }

    Py_RETURN_NONE;
}

static PyObject* python_element_get_text_content(PythonElementObject* self, void* closure)
{
    if (!self->element) {
        PyErr_SetString(PyExc_RuntimeError, "Element object is invalid");
        return nullptr;
    }

    auto text = self->element->text_content();
    if (text.has_value()) {
        auto text_str = text->to_string();
        return PyUnicode_FromString(text_str.characters());
    }

    return PyUnicode_FromString("");
}

static int python_element_set_text_content(PythonElementObject* self, PyObject* value, void* closure)
{
    if (!self->element) {
        PyErr_SetString(PyExc_RuntimeError, "Element object is invalid");
        return -1;
    }

    if (!value || !PyUnicode_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "text must be a string");
        return -1;
    }

    char const* text = PyUnicode_AsUTF8(value);
    if (!text) {
        return -1;
    }

    auto text_utf16_result = Utf16String::from_utf8(text);
    if (text_utf16_result.is_error())
        return -1;
    self->element->set_text_content(text_utf16_result.release_value());
    return 0;
}

static PyObject* python_element_get_inner_html(PythonElementObject* self, void* closure)
{
    if (!self->element) {
        PyErr_SetString(PyExc_RuntimeError, "Element object is invalid");
        return nullptr;
    }

    auto html_result = self->element->inner_html();
    if (html_result.is_error())
        return PyUnicode_FromString("");
    auto html = html_result.release_value();
    return PyUnicode_FromString(html.characters());
}

static int python_element_set_inner_html(PythonElementObject* self, PyObject* value, void* closure)
{
    if (!self->element) {
        PyErr_SetString(PyExc_RuntimeError, "Element object is invalid");
        return -1;
    }

    if (!value || !PyUnicode_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "inner_html must be a string");
        return -1;
    }

    char const* html = PyUnicode_AsUTF8(value);
    if (!html) {
        return -1;
    }

    auto html_view = StringView(html);
    auto result = self->element->set_inner_html(html_view);
    if (result.is_error()) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to set inner HTML");
        return -1;
    }
    return 0;
}

static PyMethodDef python_element_methods[] = {
    { "select", (PyCFunction)python_element_select, METH_VARARGS, "Select child elements using CSS selector" },
    { "find", (PyCFunction)python_element_find, METH_VARARGS, "Find first child element using CSS selector" },
    { "get_attribute", (PyCFunction)python_element_get_attribute, METH_VARARGS, "Get an attribute value" },
    { "set_attribute", (PyCFunction)python_element_set_attribute, METH_VARARGS, "Set an attribute value" },
    { NULL } // Sentinel
};

static PyGetSetDef python_element_getset[] = {
    { "text", (getter)python_element_get_text_content, (setter)python_element_set_text_content, "Text content of the element", NULL },
    { "html", (getter)python_element_get_inner_html, (setter)python_element_set_inner_html, "Inner HTML of the element", NULL },
    { NULL } // Sentinel
};

static PyTypeObject element_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "web.Element",               /* tp_name */
    sizeof(PythonElementObject), /* tp_basicsize */
    0,                           /* tp_itemsize */
    (destructor)python_element_dealloc, /* tp_dealloc */
    0,                           /* tp_vectorcall_offset */
    0,                           /* tp_getattr */
    0,                           /* tp_setattr */
    0,                           /* tp_as_async */
    0,                           /* tp_repr */
    0,                           /* tp_as_number */
    0,                           /* tp_as_sequence */
    0,                           /* tp_as_mapping */
    0,                           /* tp_hash */
    0,                           /* tp_call */
    0,                           /* tp_str */
    0,                           /* tp_getattro */
    0,                           /* tp_setattro */
    0,                           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,          /* tp_flags */
    "Web Element object",        /* tp_doc */
    0,                           /* tp_traverse */
    0,                           /* tp_clear */
    0,                           /* tp_richcompare */
    0,                           /* tp_weaklistoffset */
    0,                           /* tp_iter */
    0,                           /* tp_iternext */
    python_element_methods,      /* tp_methods */
    0,                           /* tp_members */
    python_element_getset,       /* tp_getset */
};

void PythonElement::setup_type()
{
    if (s_type.tp_name)
        return; // Already set up

    memcpy(&s_type, &element_type, sizeof(PyTypeObject));
    if (PyType_Ready(&s_type) < 0) {
        // Handle error appropriately
    }
}

PyObject* PythonElement::create_from_cpp_element(Web::DOM::Element& element)
{
    setup_type();

    auto* document_ptr = element.document();
    if (!document_ptr)
        return nullptr;
        
    auto* cache = document_ptr->wrapper_cache();
    if (!cache)
        document_ptr->set_wrapper_cache(make<PythonDOMWrapperCache>());

    if (auto* wrapper = document_ptr->wrapper_cache()->get_wrapper(&element))
        return wrapper;

    PythonElementObject* obj = PyObject_New(PythonElementObject, &s_type);
    if (obj) {
        obj->element = &element;
        document_ptr->wrapper_cache()->set_wrapper(&element, (PyObject*)obj);
    }
    return (PyObject*)obj;
}

Web::DOM::Element* PythonElement::get_cpp_element(PyObject* obj)
{
    if (!obj || !PyObject_TypeCheck(obj, &s_type)) {
        return nullptr;
    }
    return ((PythonElementObject*)obj)->element;
}

// PythonWindow methods
static void python_window_dealloc(PythonWindowObject* self)
{
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* python_window_get_document(PythonWindowObject* self, void* closure)
{
    if (!self->window) {
        PyErr_SetString(PyExc_RuntimeError, "Window object is invalid");
        return nullptr;
    }

    auto document_ref = self->window->document();
    if (!document_ref) {
        PyErr_SetString(PyExc_RuntimeError, "Window has no document");
        return nullptr;
    }
    return PythonDocument::create_from_cpp_document(*document_ref);
}

static PyObject* python_window_get_location(PythonWindowObject* self, void* closure)
{
    if (!self->window) {
        PyErr_SetString(PyExc_RuntimeError, "Window object is invalid");
        return nullptr;
    }

    auto location_ref = self->window->location();
    if (!location_ref) {
        PyErr_SetString(PyExc_RuntimeError, "Window has no location");
        return nullptr;
    }
    // For simplicity, return the href as a string
    auto href = location_ref->href();
    auto href_str = href.to_string();
    return PyUnicode_FromString(href_str.characters());
}

static PyGetSetDef python_window_getset[] = {
    { "document", (getter)python_window_get_document, NULL, "The document object", NULL },
    { "location", (getter)python_window_get_location, NULL, "The location object", NULL },
    { NULL } // Sentinel
};

static PyTypeObject window_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "web.Window",                /* tp_name */
    sizeof(PythonWindowObject),  /* tp_basicsize */
    0,                           /* tp_itemsize */
    (destructor)python_window_dealloc, /* tp_dealloc */
    0,                           /* tp_vectorcall_offset */
    0,                           /* tp_getattr */
    0,                           /* tp_setattr */
    0,                           /* tp_as_async */
    0,                           /* tp_repr */
    0,                           /* tp_as_number */
    0,                           /* tp_as_sequence */
    0,                           /* tp_as_mapping */
    0,                           /* tp_hash */
    0,                           /* tp_call */
    0,                           /* tp_str */
    0,                           /* tp_getattro */
    0,                           /* tp_setattro */
    0,                           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,          /* tp_flags */
    "Web Window object",         /* tp_doc */
    0,                           /* tp_traverse */
    0,                           /* tp_clear */
    0,                           /* tp_richcompare */
    0,                           /* tp_weaklistoffset */
    0,                           /* tp_iter */
    0,                           /* tp_iternext */
    0,                           /* tp_methods */
    0,                           /* tp_members */
    python_window_getset,        /* tp_getset */
};

void PythonWindow::setup_type()
{
    if (s_type.tp_name)
        return; // Already set up

    memcpy(&s_type, &window_type, sizeof(PyTypeObject));
    if (PyType_Ready(&s_type) < 0) {
        // Handle error appropriately
    }
}

PyObject* PythonWindow::create_from_cpp_window(Web::HTML::Window& window)
{
    setup_type();

    auto document_ref = window.document();
    if (!document_ref)
        return nullptr;
        
    auto* cache = document_ref->wrapper_cache();
    if (!cache)
        document_ref->set_wrapper_cache(make<PythonDOMWrapperCache>());

    if (auto* wrapper = document_ref->wrapper_cache()->get_wrapper(&window))
        return wrapper;

    PythonWindowObject* obj = PyObject_New(PythonWindowObject, &s_type);
    if (obj) {
        obj->window = &window;
        document_ref->wrapper_cache()->set_wrapper(&window, (PyObject*)obj);
    }
    return (PyObject*)obj;
}

Web::HTML::Window* PythonWindow::get_cpp_window(PyObject* obj)
{
    if (!obj || !PyObject_TypeCheck(obj, &s_type)) {
        return nullptr;
    }
    return ((PythonWindowObject*)obj)->window;
}

// Main module initialization
static PyObject* python_get_window(PyObject*, PyObject*)
{
    // This would get the current window from execution context
    // For now, return None as a placeholder
    Py_RETURN_NONE;
}

static PyMethodDef web_module_methods[] = {
    { "get_window", python_get_window, METH_NOARGS, "Get the current window object" },
    { nullptr, nullptr, 0, nullptr } // Sentinel
};

static struct PyModuleDef web_module_def = {
    PyModuleDef_HEAD_INIT,
    "web",
    "Python-friendly Web API module",
    -1,
    web_module_methods,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

bool PythonDOMAPI::initialize_module()
{
    if (s_initialized) {
        return true;
    }

    // Make sure our types are set up
    PythonDocument::setup_type();
    PythonElement::setup_type();
    PythonWindow::setup_type();

    // Initialize the test module
    TestPythonDOMModule::initialize_module();

    // Create the main web module
    s_module = PyModule_Create(&web_module_def);
    if (!s_module) {
        return false;
    }

    // Add our types to the module
    Py_INCREF(&PythonDocument::s_type);
    if (PyModule_AddObject(s_module, "Document", (PyObject*)&PythonDocument::s_type) < 0) {
        Py_DECREF(s_module);
        s_module = nullptr;
        return false;
    }

    Py_INCREF(&PythonElement::s_type);
    if (PyModule_AddObject(s_module, "Element", (PyObject*)&PythonElement::s_type) < 0) {
        Py_DECREF(s_module);
        s_module = nullptr;
        return false;
    }

    Py_INCREF(&PythonWindow::s_type);
    if (PyModule_AddObject(s_module, "Window", (PyObject*)&PythonWindow::s_type) < 0) {
        Py_DECREF(s_module);
        s_module = nullptr;
        return false;
    }

    // Add the test module
    if (auto* test_module = TestPythonDOMModule::get_module()) {
        Py_INCREF(test_module);
        if (PyModule_AddObject(s_module, "test", test_module) < 0) {
            Py_DECREF(s_module);
            s_module = nullptr;
            return false;
        }
    }

    s_initialized = true;
    return true;
}

PyObject* PythonDOMAPI::get_module()
{
    if (!s_initialized) {
        return nullptr;
    }
    return s_module;
}

}
