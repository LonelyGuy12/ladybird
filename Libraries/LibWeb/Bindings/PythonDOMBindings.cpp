/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/PythonDOMBindings.h>
#include <LibWeb/Bindings/PythonDOMWrapperCache.h>
#include <LibWeb/Bindings/PythonCompat.h>
#include <LibWeb/Bindings/TestPythonDOMModule.h>
#include <LibWeb/DOM/Document.h>
#include <LibWeb/DOM/Element.h>
#include <LibWeb/DOM/HTMLCollection.h>
#include <LibWeb/DOM/NodeList.h>
#include <LibWeb/HTML/History.h>
#include <LibWeb/HTML/Location.h>
#include <LibWeb/HTML/Window.h>
#include <LibWeb/TrustedTypes/TrustedHTML.h>
#include <LibGC/Heap.h>
#include <AK/OwnPtr.h>

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
    auto selector_view = StringView { selector, strlen(selector) };
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
            auto& element = const_cast<Web::DOM::Element&>(static_cast<Web::DOM::Element const&>(*node));
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
    auto selector_view = StringView { selector, strlen(selector) };
    auto element_result = self->document->query_selector(selector_view);
    if (element_result.is_error()) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid selector");
        return nullptr;
    }

    auto found_element = element_result.release_value();
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

    auto tag_name_view = StringView { tag_name, strlen(tag_name) };
    auto tag_name_str = MUST(String::from_utf8(tag_name_view));
    auto element = self->document->create_element(tag_name_str, Web::DOM::ElementCreationOptions{});
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
    { NULL, NULL, 0, NULL } // Sentinel
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
    0,                           /* tp_members */
    0,                           /* tp_getset */
    0,                           /* tp_base */
    0,                           /* tp_dict */
    0,                           /* tp_descr_get */
    0,                           /* tp_descr_set */
    0,                           /* tp_dictoffset */
    0,                           /* tp_init */
    0,                           /* tp_alloc */
    0,                           /* tp_new */
    0,                           /* tp_free */
    0,                           /* tp_is_gc */
    0,                           /* tp_bases */
    0,                           /* tp_mro */
    0,                           /* tp_cache */
    0,                           /* tp_subclasses */
    0,                           /* tp_weaklist */
    0,                           /* tp_del */
    0,                           /* tp_version_tag */
    0,                           /* tp_finalize */
    0,                           /* tp_vectorcall */
    0,                           /* tp_watched */
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

    if (!document.m_python_dom_wrapper_cache)
        document.m_python_dom_wrapper_cache = make<PythonDOMWrapperCache>();
    if (auto* wrapper = document.m_python_dom_wrapper_cache->get_wrapper(&document))
        return wrapper;

    PythonDocumentObject* obj = PyObject_New(PythonDocumentObject, &s_type);
    if (obj) {
        obj->document = &document;
        document.m_python_dom_wrapper_cache->set_wrapper(&document, (PyObject*)obj);
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
    auto selector_view = StringView { selector, strlen(selector) };
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
            auto& element = const_cast<Web::DOM::Element&>(static_cast<Web::DOM::Element const&>(*node));
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

    auto selector_view = StringView { selector, strlen(selector) };
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

    auto name_view = StringView { name, strlen(name) };
    auto name_str = MUST(String::from_utf8(name_view));
    auto name_fly = FlyString(name_str);
    auto attr = self->element->get_attribute(name_fly);
    if (attr.has_value()) {
        auto attr_byte_str = attr->to_byte_string();
        return PyUnicode_FromString(attr_byte_str.characters());
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

    auto name_view = StringView { name, strlen(name) };
    auto name_str = MUST(String::from_utf8(name_view));
    auto name_fly = FlyString(name_str);
    auto value_view = StringView { value, strlen(value) };
    auto value_str = MUST(String::from_utf8(value_view));
    auto result = self->element->set_attribute(name_fly, value_str);
    if (result.is_error()) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to set attribute");
        return nullptr;
    }

    Py_RETURN_NONE;
}

static PyObject* python_element_get_text_content(PythonElementObject* self, void*)
{
    if (!self->element) {
        PyErr_SetString(PyExc_RuntimeError, "Element object is invalid");
        return nullptr;
    }

    auto text = self->element->text_content();
    if (text.has_value()) {
        auto text_str = text.value();
        auto text_byte_str = text_str.to_byte_string();
        return PyUnicode_FromString(text_byte_str.characters());
    }

    return PyUnicode_FromString("");
}

static int python_element_set_text_content(PythonElementObject* self, PyObject* value, void*)
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

    auto text_view = StringView { text, strlen(text) };
    auto text_utf16 = Utf16String::from_utf8(text_view);
    auto result = self->element->set_text_content(text_utf16);
    if (result.is_error()) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to set text content");
        return -1;
    }
    return 0;
}

static PyObject* python_element_get_inner_html(PythonElementObject* self, void*)
{
    if (!self->element) {
        PyErr_SetString(PyExc_RuntimeError, "Element object is invalid");
        return nullptr;
    }

    auto html_result = self->element->inner_html();
    if (html_result.is_error())
        return PyUnicode_FromString("");
    auto html = html_result.release_value();
    // html is a Variant<TrustedHTML, Utf16String>
    Utf16String html_utf16;
    if (html.has<GC::Root<Web::TrustedTypes::TrustedHTML>>()) {
        auto& trusted = html.get<GC::Root<Web::TrustedTypes::TrustedHTML>>();
        html_utf16 = trusted->to_string();
    } else {
        html_utf16 = html.get<Utf16String>();
    }
    auto html_byte_str = html_utf16.to_byte_string();
    return PyUnicode_FromString(html_byte_str.characters());
}

static int python_element_set_inner_html(PythonElementObject* self, PyObject* value, void*)
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

    auto html_view = StringView { html, strlen(html) };
    auto html_utf16 = Utf16String::from_utf8(html_view);
    // Create TrustedHTMLOrString variant with Utf16String
    TrustedTypes::TrustedHTMLOrString html_variant = html_utf16;
    auto result = self->element->set_inner_html(html_variant);
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
    { NULL, NULL, 0, NULL } // Sentinel
};

static PyGetSetDef python_element_getset[] = {
    { "text", (getter)python_element_get_text_content, (setter)python_element_set_text_content, "Text content of the element", NULL },
    { "html", (getter)python_element_get_inner_html, (setter)python_element_set_inner_html, "Inner HTML of the element", NULL },
    { NULL, NULL, NULL, NULL, NULL } // Sentinel
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
    0,                           /* tp_base */
    0,                           /* tp_dict */
    0,                           /* tp_descr_get */
    0,                           /* tp_descr_set */
    0,                           /* tp_dictoffset */
    0,                           /* tp_init */
    0,                           /* tp_alloc */
    0,                           /* tp_new */
    0,                           /* tp_free */
    0,                           /* tp_is_gc */
    0,                           /* tp_bases */
    0,                           /* tp_mro */
    0,                           /* tp_cache */
    0,                           /* tp_subclasses */
    0,                           /* tp_weaklist */
    0,                           /* tp_del */
    0,                           /* tp_version_tag */
    0,                           /* tp_finalize */
    0,                           /* tp_vectorcall */
    0,                           /* tp_watched */
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

    auto& document_ref = element.document();
    auto* document_ptr = &document_ref;
    if (!document_ptr->m_python_dom_wrapper_cache)
        document_ptr->m_python_dom_wrapper_cache = make<PythonDOMWrapperCache>();
    if (auto* wrapper = document_ptr->m_python_dom_wrapper_cache->get_wrapper(&element))
        return wrapper;

    PythonElementObject* obj = PyObject_New(PythonElementObject, &s_type);
    if (obj) {
        obj->element = &element;
        document_ptr->m_python_dom_wrapper_cache->set_wrapper(&element, (PyObject*)obj);
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

static PyObject* python_window_get_document(PythonWindowObject* self, void*)
{
    if (!self->window) {
        PyErr_SetString(PyExc_RuntimeError, "Window object is invalid");
        return nullptr;
    }

    auto document_ref = self->window->document();
    if (!document_ref.ptr()) {
        PyErr_SetString(PyExc_RuntimeError, "Window has no document");
        return nullptr;
    }
    return PythonDocument::create_from_cpp_document(const_cast<Web::DOM::Document&>(*document_ref));
}

static PyObject* python_window_get_location(PythonWindowObject* self, void*)
{
    if (!self->window) {
        PyErr_SetString(PyExc_RuntimeError, "Window object is invalid");
        return nullptr;
    }

    auto location_ref = self->window->location();
    if (!location_ref.ptr()) {
        PyErr_SetString(PyExc_RuntimeError, "Window has no location");
        return nullptr;
    }
    // For simplicity, return the href as a string
    auto href_result = location_ref->href();
    if (href_result.is_error())
        return PyUnicode_FromString("");
    auto href = href_result.release_value();
    auto href_byte_str = href.to_byte_string();
    return PyUnicode_FromString(href_byte_str.characters());
}

static PyGetSetDef python_window_getset[] = {
    { "document", (getter)python_window_get_document, NULL, "The document object", NULL },
    { "location", (getter)python_window_get_location, NULL, "The location object", NULL },
    { NULL, NULL, NULL, NULL, NULL } // Sentinel
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
    0,                           /* tp_base */
    0,                           /* tp_dict */
    0,                           /* tp_descr_get */
    0,                           /* tp_descr_set */
    0,                           /* tp_dictoffset */
    0,                           /* tp_init */
    0,                           /* tp_alloc */
    0,                           /* tp_new */
    0,                           /* tp_free */
    0,                           /* tp_is_gc */
    0,                           /* tp_bases */
    0,                           /* tp_mro */
    0,                           /* tp_cache */
    0,                           /* tp_subclasses */
    0,                           /* tp_weaklist */
    0,                           /* tp_del */
    0,                           /* tp_version_tag */
    0,                           /* tp_finalize */
    0,                           /* tp_vectorcall */
    0,                           /* tp_watched */
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
    if (!document_ref.ptr())
        return nullptr;
    
    // Cast away const to access mutable wrapper cache    
    auto* document_mut = const_cast<Web::DOM::Document*>(document_ref.ptr());
    if (!document_mut->m_python_dom_wrapper_cache)
        document_mut->m_python_dom_wrapper_cache = make<PythonDOMWrapperCache>();
    if (auto* wrapper = document_mut->m_python_dom_wrapper_cache->get_wrapper(&window))
        return wrapper;

    PythonWindowObject* obj = PyObject_New(PythonWindowObject, &s_type);
    if (obj) {
        obj->window = &window;
        document_mut->m_python_dom_wrapper_cache->set_wrapper(&window, (PyObject*)obj);
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
