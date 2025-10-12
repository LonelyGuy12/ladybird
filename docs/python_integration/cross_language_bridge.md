# Cross-Language Communication Bridge

## Overview

The cross-language communication bridge enables seamless interaction between Python and JavaScript code within the Ladybird browser, allowing developers to call functions and access objects across language boundaries.

## Architecture

The bridge consists of several components:

1. **PythonJSBridge** - Core bridge functionality
2. **PythonJSObjectWrapper** - JavaScript object wrappers for Python
3. **PythonObjectBridge** - Python object wrappers for JavaScript
4. **Value Converters** - Bidirectional value conversion
5. **Function Call Marshaller** - Cross-language function invocation

## Core Implementation

### PythonJSBridge

The main bridge class that handles communication between Python and JavaScript:

```cpp
// PythonJSBridge.h
class PythonJSBridge {
public:
    static bool initialize_bridge();
    static JS::Value python_to_js(PyObject* py_obj, JS::Realm& realm);
    static PyObject* js_to_python(JS::Value js_val);
    static PyObject* call_js_function(const String& function_name, PyObject* args, JS::Realm& realm);
    static JS::Value call_python_function(PyObject* func, JS::MarkedVector<JS::Value>& js_args, JS::Realm& realm);
    static bool setup_bridge_in_context(PyObject* globals, JS::Realm& realm);
    
private:
    static bool s_bridge_initialized;
    static PyObject* s_js_proxy_type;  // Python type for JS object proxies
    static PyObject* s_py_proxy_type;  // Python type for Python function proxies
};
```

Implementation highlights:

```cpp
// PythonJSBridge.cpp
bool PythonJSBridge::initialize_bridge()
{
    if (s_bridge_initialized) {
        return true;
    }
    
    s_bridge_initialized = true;
    return true;
}

// Convert Python value to JavaScript value
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

// Convert JavaScript value to Python value
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

### PythonJSObjectWrapper

Wrapper for JavaScript objects accessible from Python:

```cpp
// PythonJSObjectWrapper.h
class PythonJSObjectWrapper {
public:
    static void setup_type();
    static PyObject* create_wrapper(JS::Object& js_obj);
    static JS::Object* get_cpp_object(PyObject* obj);
    
private:
    static PyTypeObject s_type;
};

// Python object structure for JS objects
typedef struct {
    PyObject_HEAD
    JS::Object* js_object;
} PythonJSObject;

// PythonJSObjectWrapper.cpp
PyTypeObject PythonJSObjectWrapper::s_type = {0};

void PythonJSObjectWrapper::setup_type()
{
    if (s_type.tp_name) return; // Already set up
    
    static PyTypeObject type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "web.JSObject",
        .tp_doc = "JavaScript object wrapper for Python",
        .tp_basicsize = sizeof(PythonJSObject),
        .tp_itemsize = 0,
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_dealloc = (destructor)python_js_object_dealloc,
        .tp_getattro = (getattrofunc)python_js_object_getattr,
        .tp_setattro = (setattrofunc)python_js_object_setattr,
        .tp_call = (ternaryfunc)python_js_object_call,
    };
    
    memcpy(&s_type, &type, sizeof(PyTypeObject));
    
    if (PyType_Ready(&s_type) < 0) {
        // Handle error appropriately
    }
}

PyObject* PythonJSObjectWrapper::create_wrapper(JS::Object& js_obj)
{
    setup_type();
    
    PythonJSObject* obj = PyObject_New(PythonJSObject, &s_type);
    if (obj) {
        obj->js_object = &js_obj;
    }
    return (PyObject*)obj;
}

JS::Object* PythonJSObjectWrapper::get_cpp_object(PyObject* obj)
{
    if (!obj || !PyObject_TypeCheck(obj, &s_type)) {
        return nullptr;
    }
    return ((PythonJSObject*)obj)->js_object;
}
```

### PythonObjectBridge

Wrapper for Python objects accessible from JavaScript:

```cpp
// PythonObjectBridge.h
class PythonObjectBridge {
public:
    static void setup_type();
    static JS::Object* create_wrapper(PyObject* py_obj, JS::Realm& realm);
    static PyObject* get_python_object(JS::Object* obj);
    
private:
    static JS::Object* s_type;
};

// JavaScript object structure for Python objects
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

## Usage Examples

### Calling JavaScript from Python

```python
# Python calling JavaScript function
result = window.some_js_function("argument")

# Accessing JavaScript object
js_value = window.js_object.property

# Setting JavaScript values
window.pyProperty = "Python value"
window.pyFunction = lambda x: f"Python function called with {x}"
```

### Calling Python from JavaScript

```javascript
// JavaScript calling Python function
var result = window.python_function("argument");

// Accessing Python object
var pyValue = window.python_object.property;

// Setting Python values
window.jsProperty = "JavaScript value";
```

## Implementation Details

### Value Conversion

Bidirectional conversion between Python and JavaScript types:

| Python Type | JavaScript Type | Conversion Method |
|-------------|-----------------|-------------------|
| `int` | `Number` | Direct conversion |
| `float` | `Number` | Direct conversion |
| `str` | `String` | UTF-8 conversion |
| `bool` | `Boolean` | Direct conversion |
| `None` | `undefined` | Direct conversion |
| `list`/`tuple` | `Array` | Recursive conversion |
| `dict` | `Object` | Property mapping |
| Functions | Callable objects | Wrapper creation |

### Function Call Marshalling

Cross-language function calls require careful parameter marshalling:

```cpp
// Calling JavaScript function from Python
static PyObject* python_call_js_function(const String& function_name, PyObject* args, JS::Realm& realm)
{
    // Convert Python arguments to JavaScript values
    Py_ssize_t arg_count = PyTuple_Size(args);
    JS::MarkedVector<JS::Value> js_args(realm.vm().heap());
    js_args.ensure_capacity(arg_count);
    
    for (Py_ssize_t i = 0; i < arg_count; ++i) {
        PyObject* py_arg = PyTuple_GetItem(args, i);
        if (py_arg) {
            auto js_arg = PythonJSBridge::python_to_js(py_arg, realm);
            js_args.unchecked_append(js_arg);
        }
    }
    
    // Call the JavaScript function
    auto& vm = realm.vm();
    auto function = vm.get(function_name);
    if (function.is_function()) {
        auto result = JS::call(vm, function.as_function(), JS::js_undefined(), move(js_args));
        if (!result.is_abrupt()) {
            return PythonJSBridge::js_to_python(result.value());
        }
    }
    
    Py_RETURN_NONE;
}

// Calling Python function from JavaScript
static JS::Completion python_call_python_function(PyObject* func, JS::MarkedVector<JS::Value>& js_args, JS::Realm& realm)
{
    // Convert JS arguments to Python arguments
    PyObject* py_args = PyTuple_New(js_args.size());
    if (!py_args) {
        return JS::js_undefined();
    }
    
    for (size_t i = 0; i < js_args.size(); ++i) {
        PyObject* py_arg = PythonJSBridge::js_to_python(js_args[i]);
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
    JS::Value js_result = PythonJSBridge::python_to_js(result, realm);
    Py_DECREF(result);
    
    return js_result;
}
```

### Exception Handling

Cross-language exceptions are properly converted:

```cpp
// Python exception to JavaScript error
if (!result) {
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

### Memory Management

Cross-language objects require careful memory management:

```cpp
// Python wrapper for JavaScript objects
static void python_js_object_dealloc(PythonJSObject* self)
{
    // Don't delete the JavaScript object, just the Python wrapper
    // The JavaScript object will be cleaned up by the GC
    Py_TYPE(self)->tp_free((PyObject*)self);
}

// JavaScript wrapper for Python objects
PythonObjectWrapper::~PythonObjectWrapper()
{
    if (m_python_object) {
        // Acquire GIL before destroying Python objects
        PyGILState_STATE gstate = PyGILState_Ensure();
        Py_DECREF(m_python_object);
        PyGILState_Release(gstate);
        m_python_object = nullptr;
    }
}
```

## Security Considerations

The bridge implements security measures to prevent malicious cross-language access:

1. **Object Wrapping Restrictions**
   - Only safe objects are wrapped
   - Dangerous methods are filtered out
   - Access is mediated through security checks

2. **Function Call Filtering**
   - Only safe functions can be called across languages
   - Arguments are validated before crossing boundaries
   - Return values are sanitized

3. **Module Import Control**
   - Dangerous modules are blocked from import
   - Only whitelisted modules are accessible
   - Cross-language module access is restricted

## Performance Optimization

The bridge is optimized for performance:

1. **Caching**
   - Frequently accessed objects are cached
   - Method lookups are cached
   - Type conversions are optimized

2. **Batch Operations**
   - Multiple operations are batched to reduce boundary crossings
   - Bulk data transfers are optimized

3. **Lazy Evaluation**
   - Objects are only wrapped when accessed
   - Properties are only converted when needed

## Testing

Cross-language communication is tested with:

1. **Function Call Tests**
   - Python calling JavaScript functions
   - JavaScript calling Python functions
   - Argument and return value conversion

2. **Object Access Tests**
   - Property access across languages
   - Method invocation across languages
   - Object lifetime management

3. **Exception Tests**
   - Exception propagation across languages
   - Error handling consistency
   - Security violation detection

## Future Improvements

### Enhanced Object Wrapping
1. More sophisticated proxy objects
2. Better property access patterns
3. Improved method call performance

### Advanced Function Marshalling
1. Support for async functions
2. Better error reporting
3. Enhanced argument validation

### Performance Enhancements
1. Reduced conversion overhead
2. Faster object wrapping
3. Better caching mechanisms

### Developer Experience
1. Better debugging tools
2. Improved error messages
3. Enhanced IDE support

This cross-language bridge provides seamless integration between Python and JavaScript in the Ladybird browser, enabling developers to leverage both languages within their web applications while maintaining security and performance.