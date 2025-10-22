# Python DOM API Bindings Implementation

## Overview

The Python DOM API bindings provide an idiomatic Python interface to web APIs, making it easy for Python developers to work with the DOM in a familiar way.

## Core Principles

1. **Pythonic Syntax**: APIs follow Python naming conventions (snake_case)
2. **Intuitive Methods**: Common operations have simple, clear method names
3. **Consistent Behavior**: Methods behave like their JavaScript counterparts but with Python semantics
4. **Error Handling**: Exceptions follow Python conventions
5. **Type Safety**: Appropriate Python types are used for DOM values

## Core Bindings Structure

### PythonDocument Class
Located in `Libraries/LibWeb/Bindings/PythonDOMBindings.cpp`

Wrapper for DOM Document with Python-friendly API:

```cpp
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
```

### PythonDocument Methods

#### `select(selector)`
Query multiple elements using CSS selector syntax (Python-idiomatic):

```python
# Python code
elements = document.select(".my-class")
```

Implementation:
```cpp
static PyObject* python_document_select(PythonDocumentObject* self, PyObject* args)
{
    if (!self->document) {
        PyErr_SetString(PyExc_RuntimeError, "Document object is invalid");
        return nullptr;
    }
    
    const char* selector = nullptr;
    if (!PyArg_ParseTuple(args, "s", &selector)) {
        return nullptr;
    }
    
    // Use the document's querySelectorAll method (converted to Python API)
    auto elements = self->document->query_selector_all(selector);
    if (elements.is_error()) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid selector");
        return nullptr;
    }
    
    // Create a Python list to hold the results
    PyObject* result = PyList_New(0);
    if (!result) return nullptr;
    
    auto node_list = elements.release_value();
    for (size_t i = 0; i < node_list->length(); ++i) {
        auto node = node_list->item(i);
        if (node && is<Web::DOM::Element>(*node)) {
            auto element = &to<Web::DOM::Element>(*node);
            PyObject* element_wrapper = PythonElement::create_from_cpp_element(*element);
            if (element_wrapper) {
                PyList_Append(result, element_wrapper);
                Py_DECREF(element_wrapper);
            }
        }
    }
    
    return result;
}
```

#### `find(selector)`
Query single element using CSS selector syntax (Python-idiomatic):

```python
# Python code
element = document.find("#my-id")
```

Implementation:
```cpp
static PyObject* python_document_find(PythonDocumentObject* self, PyObject* args)
{
    if (!self->document) {
        PyErr_SetString(PyExc_RuntimeError, "Document object is invalid");
        return nullptr;
    }
    
    const char* selector = nullptr;
    if (!PyArg_ParseTuple(args, "s", &selector)) {
        return nullptr;
    }
    
    // Use the document's querySelector method (converted to Python API)
    auto element = self->document->query_selector(selector);
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
```

#### `create_element(tag_name)`
Create a new DOM element:

```python
# Python code
new_div = document.create_element("div")
```

Implementation:
```cpp
static PyObject* python_document_create_element(PythonDocumentObject* self, PyObject* args)
{
    if (!self->document) {
        PyErr_SetString(PyExc_RuntimeError, "Document object is invalid");
        return nullptr;
    }
    
    const char* tag_name = nullptr;
    if (!PyArg_ParseTuple(args, "s", &tag_name)) {
        return nullptr;
    }
    
    auto element = self->document->create_element(tag_name);
    if (element.is_error()) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid element tag name");
        return nullptr;
    }
    
    return PythonElement::create_from_cpp_element(*element.release_value());
}
```

### PythonElement Methods

#### `text` property
Get/set text content of element:

```python
# Python code
text_content = element.text
element.text = "New text content"
```

Implementation:
```cpp
static PyObject* python_element_get_text_content(PythonElementObject* self, void* closure)
{
    if (!self->element) {
        PyErr_SetString(PyExc_RuntimeError, "Element object is invalid");
        return nullptr;
    }
    
    auto text = self->element->text_content();
    if (text.has_value()) {
        return PyUnicode_FromString(text->to_utf8().characters());
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
    
    const char* text = PyUnicode_AsUTF8(value);
    if (!text) {
        return -1;
    }
    
    self->element->set_text_content(text);
    return 0;
}
```

#### `html` property
Get/set HTML content of element:

```python
# Python code
html_content = element.html
element.html = "<p>New HTML content</p>"
```

Implementation:
```cpp
static PyObject* python_element_get_inner_html(PythonElementObject* self, void* closure)
{
    if (!self->element) {
        PyErr_SetString(PyExc_RuntimeError, "Element object is invalid");
        return nullptr;
    }
    
    auto html = self->element->inner_html();
    return PyUnicode_FromString(html.to_utf8().characters());
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
    
    const char* html = PyUnicode_AsUTF8(value);
    if (!html) {
        return -1;
    }
    
    self->element->set_inner_html(html);
    return 0;
}
```

#### `get_attribute(name)`
Get element attribute value:

```python
# Python code
class_value = element.get_attribute("class")
```

Implementation:
```cpp
static PyObject* python_element_get_attribute(PythonElementObject* self, PyObject* args)
{
    if (!self->element) {
        PyErr_SetString(PyExc_RuntimeError, "Element object is invalid");
        return nullptr;
    }
    
    const char* name = nullptr;
    if (!PyArg_ParseTuple(args, "s", &name)) {
        return nullptr;
    }
    
    auto value = self->element->get_attribute(name);
    if (value.has_value()) {
        return PyUnicode_FromString(value->characters());
    }
    
    Py_RETURN_NONE;
}
```

#### `set_attribute(name, value)`
Set element attribute value:

```python
# Python code
element.set_attribute("class", "new-class")
```

Implementation:
```cpp
static PyObject* python_element_set_attribute(PythonElementObject* self, PyObject* args)
{
    if (!self->element) {
        PyErr_SetString(PyExc_RuntimeError, "Element object is invalid");
        return nullptr;
    }
    
    const char* name = nullptr;
    const char* value = nullptr;
    if (!PyArg_ParseTuple(args, "ss", &name, &value)) {
        return nullptr;
    }
    
    self->element->set_attribute(name, value);
    
    Py_RETURN_NONE;
}
```

### PythonWindow Methods

#### `alert(message)`
Display alert dialog:

```python
# Python code
window.alert("Hello from Python!")
```

Implementation:
```cpp
static PyObject* python_window_alert(PythonWindowObject* self, PyObject* args)
{
    if (!self->window) {
        PyErr_SetString(PyExc_RuntimeError, "Window object is invalid");
        return nullptr;
    }
    
    const char* message = nullptr;
    if (!PyArg_ParseTuple(args, "s", &message)) {
        return nullptr;
    }
    
    auto utf16_message = Utf16String::from_utf8(message).release_value_but_fixme_should_propagate_errors();
    self->window->alert(utf16_message);
    
    Py_RETURN_NONE;
}
```

#### `document` property
Access the current document:

```python
# Python code
doc = window.document
```

Implementation:
```cpp
static PyObject* python_window_get_document(PythonWindowObject* self, void* closure)
{
    if (!self->window) {
        PyErr_SetString(PyExc_RuntimeError, "Window object is invalid");
        return nullptr;
    }
    
    auto& document = self->window->document();
    return PythonDocument::create_from_cpp_document(document);
}
```

## Cross-Language Object Access

### JavaScript Objects in Python

JavaScript objects accessed from Python are wrapped in proxy objects that forward method calls:

```cpp
// PythonJSObjectWrapper.cpp
PyObject* PythonJSObjectWrapper::create_wrapper(JS::Object& js_obj)
{
    setup_type();
    
    PythonJSObjectWrapper* wrapper = PyObject_New(PythonJSObjectWrapper, &s_type);
    if (wrapper) {
        wrapper->js_object_ptr = &js_obj;
    }
    return (PyObject*)wrapper;
}

JS::Object* PythonJSObjectWrapper::get_js_object(PyObject* wrapper)
{
    if (!wrapper || !PyObject_TypeCheck(wrapper, &s_type)) {
        return nullptr;
    }
    
    PythonJSObjectWrapper* js_wrapper = (PythonJSObjectWrapper*)wrapper;
    return static_cast<JS::Object*>(js_wrapper->js_object_ptr);
}
```

Attribute access:
```cpp
static PyObject* python_js_object_getattr(PythonJSObjectWrapper* self, PyObject* attr_name)
{
    if (!self->js_object_ptr) {
        PyErr_SetString(PyExc_RuntimeError, "JavaScript object is invalid");
        return nullptr;
    }
    
    if (PyUnicode_Check(attr_name)) {
        const char* attr_str = PyUnicode_AsUTF8(attr_name);
        if (!attr_str) {
            return nullptr;
        }
        
        auto& vm = self->js_object_ptr->shape().realm().vm();
        auto js_key = JS::PropertyKey::symbol_or_string(vm, attr_str);
        auto js_value = self->js_object_ptr->get(js_key);
        
        if (js_value.is_error()) {
            PyErr_SetString(PyExc_AttributeError, attr_str);
            return nullptr;
        }
        
        return PythonJSBridge::js_to_python(js_value.release_value());
    }
    
    PyErr_SetString(PyExc_TypeError, "Attribute name must be a string");
    return nullptr;
}
```

### Python Objects in JavaScript

Python objects accessible from JavaScript are wrapped in proxy objects:

```cpp
// PythonObjectBridge.cpp
JS::Object* PythonObjectBridge::create_wrapper(PyObject* py_obj, JS::Realm& realm)
{
    setup_type();
    
    auto& vm = realm.vm();
    auto wrapper = vm.heap().allocate<PythonObjectWrapper>(py_obj, realm);
    return wrapper;
}

PyObject* PythonObjectBridge::get_python_object(JS::Object* js_obj)
{
    if (!js_obj || !is<PythonObjectWrapper>(*js_obj)) {
        return nullptr;
    }
    
    auto& wrapper = static_cast<PythonObjectWrapper&>(*js_obj);
    return wrapper.python_object();
}
```

## Value Conversion Between Languages

### Python to JavaScript Conversion

```cpp
// PythonJSBridge.cpp
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
```

### JavaScript to Python Conversion

```cpp
// PythonJSBridge.cpp
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
```

## Method Call Marshalling

### Calling JavaScript Functions from Python

```cpp
// PythonJSBridge.cpp
PyObject* PythonJSBridge::call_js_function(const String& function_name, PyObject* args, JS::Realm& realm)
{
    // Convert Python arguments to JavaScript values
    Py_ssize_t arg_count = PyTuple_Size(args);
    JS::MarkedVector<JS::Value> js_args(realm.vm().heap());
    js_args.ensure_capacity(arg_count);
    
    for (Py_ssize_t i = 0; i < arg_count; ++i) {
        PyObject* py_arg = PyTuple_GetItem(args, i);
        if (py_arg) {
            auto js_value = python_to_js(py_arg, realm);
            js_args.unchecked_append(js_value);
        }
    }
    
    // Call the JavaScript function
    auto& vm = realm.vm();
    auto function = vm.get(function_name);
    if (function.is_function()) {
        auto result = JS::call(vm, function.as_function(), JS::js_undefined(), move(js_args));
        if (!result.is_abrupt()) {
            return js_to_python(result.value());
        }
    }
    
    Py_RETURN_NONE;
}
```

### Calling Python Functions from JavaScript

```cpp
// PythonJSBridge.cpp
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
```

## Object Proxy Implementation

### Python Object Proxy for JavaScript

```cpp
// PythonObjectBridge.cpp
class PythonObjectWrapper final : public JS::Object {
    JS_OBJECT(PythonObjectWrapper, JS::Object);
    GC_DECLARE_ALLOCATOR(PythonObjectWrapper);

public:
    virtual ~PythonObjectWrapper() override;

    PyObject* python_object() { return m_python_object; }
    PyObject const* python_object() const { return m_python_object; }

    static GC::Ref<PythonObjectWrapper> create(PyObject* py_obj, JS::Realm& realm);

    virtual JS::ThrowCompletionOr<JS::Value> internal_get(JS::PropertyKey const&, JS::Value receiver) override;
    virtual JS::ThrowCompletionOr<bool> internal_set(JS::PropertyKey const&, JS::Value value, JS::Value receiver) override;
    virtual JS::ThrowCompletionOr<JS::Value> internal_call(JS::MarkedVector<JS::Value>& arguments) override;

private:
    PythonObjectWrapper(PyObject* py_obj, JS::Realm& realm);

    virtual bool is_python_object_wrapper() const final { return true; }

    virtual void visit_edges(Cell::Visitor&) override;

    PyObject* m_python_object { nullptr };
};
```

### JavaScript Object Proxy for Python

```cpp
// PythonJSObjectWrapper.cpp
typedef struct {
    PyObject_HEAD
    void* js_object_ptr;  // Will point to JS::Object*
} PythonJSObjectWrapper;

static PyObject* python_js_object_getattr(PythonJSObjectWrapper* self, PyObject* attr_name);
static int python_js_object_setattr(PythonJSObjectWrapper* self, PyObject* attr_name, PyObject* value);
static PyObject* python_js_object_call(PythonJSObjectWrapper* self, PyObject* args, PyObject* kwargs);
```

## Security Considerations

### Restricted Built-ins

Dangerous Python built-ins are removed:

```cpp
// PythonSecurityModel.cpp
bool PythonSecurityModel::restrict_builtins(PyObject* globals)
{
    if (!globals || !PyDict_Check(globals)) {
        return false;
    }
    
    PyObject* builtins = PyEval_GetBuiltins();
    if (!builtins) {
        return false;
    }
    
    // Create a restricted builtins dictionary
    PyObject* restricted_builtins = PyDict_New();
    if (!restricted_builtins) {
        return false;
    }
    
    // Allow safe built-in functions
    const char* safe_builtins[] = {
        "abs", "all", "any", "bool", "chr", "dict", "enumerate", "filter", "float", 
        "int", "isinstance", "len", "list", "map", "max", "min", "ord", "pow", 
        "range", "sorted", "str", "sum", "type", "zip", "True", "False", "None"
    };
    
    for (const char* builtin_name : safe_builtins) {
        PyObject* builtin_func = PyDict_GetItemString(builtins, builtin_name);
        if (builtin_func) {
            PyDict_SetItemString(restricted_builtins, builtin_name, builtin_func);
        }
    }
    
    // Replace the __builtins__ in the globals
    PyDict_SetItemString(globals, "__builtins__", restricted_builtins);
    Py_DECREF(restricted_builtins);
    
    return true;
}
```

### Module Import Control

Only safe modules are allowed to be imported:

```cpp
// PythonSecurityModel.cpp
bool PythonSecurityModel::should_allow_module_import(const String& module_name, const URL& origin)
{
    // For security, only allow imports for specific modules that are safe
    // In a real implementation, we'd have a more sophisticated whitelist
    StringView module_view = module_name;
    
    // Allow only safe modules
    return module_view.is_one_of("json"sv, "datetime"sv, "math"sv, "random"sv, "re"sv, "collections"sv, "itertools"sv);
}
```

## Performance Optimization

### Caching Mechanisms

Cache frequently accessed objects:

```cpp
// PythonDOMBindings.cpp
class PythonDocument {
private:
    static HashMap<String, PyObject*> s_method_cache;
    
public:
    static PyObject* get_cached_method(const String& name) {
        if (auto it = s_method_cache.find(name); it != s_method_cache.end()) {
            return it->value;
        }
        
        // Look up method and cache it
        PyObject* method = PyObject_GetAttrString(s_type, name.characters());
        if (method) {
            s_method_cache.set(name, method);
            return method;
        }
        
        return nullptr;
    }
};
```

### Efficient Value Conversion

Minimize unnecessary conversions:

```cpp
// PythonJSBridge.cpp
static StringView get_string_view_from_python(PyObject* py_str) {
    if (PyUnicode_Check(py_str)) {
        Py_ssize_t size;
        const char* data = PyUnicode_AsUTF8AndSize(py_str, &size);
        if (data) {
            // Create a StringView without copying
            return StringView { data, static_cast<size_t>(size) };
        }
    }
    return {};
}
```

## Error Handling

### Cross-Language Exceptions

Proper error conversion between languages:

```cpp
// PythonScript.cpp
if (!result) {
    // Python error occurred
    PyObject* error_type = nullptr;
    PyObject* error_value = nullptr;
    PyObject* error_traceback = nullptr;
    
    PyErr_Fetch(&error_type, &error_value, &error_traceback);
    PyErr_NormalizeException(&error_type, &error_value, &error_traceback);
    
    if (error_value) {
        PyObject* error_str = PyObject_Str(error_value);
        if (error_str) {
            const char* error_cstr = PyUnicode_AsUTF8(error_str);
            if (error_cstr) {
                auto error_message = String::from_utf8(error_cstr).value_or("Unknown Python error"sv);
                evaluation_status = JS::throw_completion(JS::Error::create(realm, JS::ErrorType::Generic, error_message));
            }
            Py_DECREF(error_str);
        }
        Py_DECREF(error_value);
    }
    if (error_type) Py_DECREF(error_type);
    if (error_traceback) Py_DECREF(error_traceback);
}
```

## Testing

### Unit Tests

Test individual API methods:

```python
# test_document_api.py
def test_document_select():
    # Create test elements
    div1 = document.create_element("div")
    div1.set_attribute("class", "test-class")
    div2 = document.create_element("div")
    div2.set_attribute("class", "test-class")
    
    # Append to document
    document.body.append_child(div1)
    document.body.append_child(div2)
    
    # Test select method
    elements = document.select(".test-class")
    assert len(elements) == 2
    assert isinstance(elements[0], Element)
    assert isinstance(elements[1], Element)

def test_document_find():
    # Create test element with ID
    div = document.create_element("div")
    div.set_attribute("id", "test-id")
    document.body.append_child(div)
    
    # Test find method
    element = document.find("#test-id")
    assert element is not None
    assert isinstance(element, Element)
    assert element.get_attribute("id") == "test-id"

def test_element_text_property():
    # Create test element
    div = document.create_element("div")
    
    # Test text property
    div.text = "Hello, World!"
    assert div.text == "Hello, World!"

def test_element_html_property():
    # Create test element
    div = document.create_element("div")
    
    # Test html property
    div.html = "<p>Hello, <strong>World!</strong></p>"
    assert "<p>Hello, <strong>World!</strong></p>" in div.html

def test_element_attributes():
    # Create test element
    div = document.create_element("div")
    
    # Test attribute methods
    div.set_attribute("class", "test-class")
    assert div.get_attribute("class") == "test-class"
    
    div.remove_attribute("class")
    assert div.get_attribute("class") is None
```

### Integration Tests

Test complete functionality:

```python
# test_full_integration.py
def test_full_python_script_execution():
    # Test complete Python script execution in browser context
    
    # Create output div
    output = document.create_element("div")
    output.set_attribute("id", "python-output")
    document.body.append_child(output)
    
    # Execute Python code that modifies DOM
    document.find("#python-output").text = "Modified by Python"
    
    # Verify modification
    assert document.find("#python-output").text == "Modified by Python"
    
    # Test window access
    assert window.document is not None
    assert window.location is not None
    
    # Test console logging
    print("Python script executed successfully")
    
    # Test element creation and manipulation
    new_element = document.create_element("p")
    new_element.text = "Dynamically created by Python"
    document.body.append_child(new_element)
    
    # Verify element was created
    assert document.find("p").text == "Dynamically created by Python"

def test_cross_language_communication():
    # Test communication between Python and JavaScript
    
    # Define Python function accessible from JavaScript
    def python_function(arg):
        return f"Python received: {arg}"
    
    # Make it available globally
    window.python_function = python_function
    
    # JavaScript would call this as:
    # var result = window.python_function("test");
    # assert(result === "Python received: test");
```

## Future Improvements

### Extended API Coverage
1. Complete CSS API bindings
2. Full Web API implementation (Fetch, WebSockets, etc.)
3. Enhanced event handling
4. Better error reporting

### Performance Enhancements
1. Faster value conversion
2. Better caching mechanisms
3. Reduced cross-language overhead
4. Optimized object proxying

### Developer Experience
1. Better debugging tools
2. IDE integration
3. Code completion support
4. Documentation generation

This comprehensive DOM API binding system provides Python developers with a familiar and powerful interface to web APIs while maintaining the security and performance requirements of a browser engine.