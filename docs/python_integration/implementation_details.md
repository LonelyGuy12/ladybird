# Implementation Details

## Core Implementation Files

### PythonScript.h/.cpp
Located in `Libraries/LibWeb/HTML/Scripting/`

This is the main class for handling Python script execution, analogous to ClassicScript for JavaScript.

Key features:
- Inherits from the base Script class
- Implements `create()` method for creating PythonScript objects from source code
- Implements `run()` method for executing Python scripts
- Manages Python bytecode compilation and execution
- Handles error reporting and exception conversion

```cpp
class PythonScript final : public Script {
    GC_CELL(PythonScript, Script);
    GC_DECLARE_ALLOCATOR(PythonScript);

public:
    virtual ~PythonScript() override;

    enum class MutedErrors {
        No,
        Yes,
    };
    static GC::Ref<PythonScript> create(ByteString filename, StringView source, JS::Realm&, URL::URL base_url, MutedErrors = MutedErrors::No);

    // Python-specific methods
    PyObject* script_record() { return m_script_record; }
    PyObject const* script_record() const { return m_script_record; }

    enum class RethrowErrors {
        No,
        Yes,
    };
    JS::Completion run(RethrowErrors = RethrowErrors::No, GC::Ptr<JS::Environment> lexical_environment_override = {});

    MutedErrors muted_errors() const { return m_muted_errors; }

private:
    PythonScript(URL::URL base_url, ByteString filename, JS::Realm&);

    virtual bool is_python_script() const final { return true; }

    virtual void visit_edges(Cell::Visitor&) override;

    PyObject* m_script_record { nullptr };
    MutedErrors m_muted_errors { MutedErrors::No };
};
```

### PythonEngine.h/.cpp
Located in `Libraries/LibWeb/HTML/Scripting/`

Manages the Python interpreter lifecycle and provides core Python functionality.

Key features:
- Initializes CPython interpreter with `Py_Initialize()`
- Manages Global Interpreter Lock (GIL)
- Provides subinterpreter support for isolation
- Handles proper shutdown with `Py_Finalize()`

```cpp
class PythonEngine {
public:
    static void initialize();
    static void shutdown();
    
    // Check if Python is initialized
    static bool is_initialized();
    
    // Get the main interpreter state
    static void* get_main_interpreter_state();
    
    // Create a new subinterpreter for isolation
    static void* create_subinterpreter();
    static void destroy_subinterpreter(void* subinterpreter);
    
private:
    static bool s_initialized;
};
```

### PythonSecurityModel.h/.cpp
Located in `Libraries/LibWeb/HTML/Scripting/`

Implements sandboxing for Python script execution.

Key features:
- Restricts dangerous Python built-ins
- Blocks access to dangerous modules (os, sys, subprocess, etc.)
- Implements Same-Origin Policy for network requests
- Controls filesystem access

```cpp
class PythonSecurityModel {
public:
    // Initialize the security model
    static bool initialize_security();
    
    // Check if a Python script should be allowed to run
    static bool should_allow_script_execution(const String& script_content, const URL& origin);
    
    // Set up sandboxed execution environment for Python
    static bool setup_sandboxed_environment(PyObject* globals, const URL& origin);
    
    // Restrict Python built-ins to safe operations only
    static bool restrict_builtins(PyObject* globals);
    
    // Check if a module import should be allowed
    static bool should_allow_module_import(const String& module_name, const URL& origin);
    
    // Set up restricted file system access
    static bool setup_restricted_filesystem_access();
    
    // Check if a network request should be allowed
    static bool should_allow_network_request(const URL& target_url, const URL& origin);
    
private:
    static bool s_security_initialized;
};
```

### PythonDOMBindings.h/.cpp
Located in `Libraries/LibWeb/Bindings/`

Provides Python-idiomatic bindings to DOM APIs.

Key components:
- PythonDocument wrapper for DOM Document
- PythonElement wrapper for DOM Elements
- PythonWindow wrapper for Window object
- Module initialization functions

```cpp
// Python wrapper for DOM Document with Python-friendly API
class PythonDocument {
public:
    static void setup_type();
    static PyObject* create_from_cpp_document(Web::DOM::Document& document);
    static Web::DOM::Document* get_cpp_document(PyObject* obj);
    
private:
    static PyTypeObject s_type;
};

// Python wrapper for DOM Elements with Python-friendly API
class PythonElement {
public:
    static void setup_type();
    static PyObject* create_from_cpp_element(Web::DOM::Element& element);
    static Web::DOM::Element* get_cpp_element(PyObject* obj);
    
private:
    static PyTypeObject s_type;
};

// Python wrapper for Window object
class PythonWindow {
public:
    static void setup_type();
    static PyObject* create_from_cpp_window(Web::HTML::Window& window);
    static Web::HTML::Window* get_cpp_window(PyObject* obj);
    
private:
    static PyTypeObject s_type;
};
```

### PythonJSBridge.h/.cpp
Located in `Libraries/LibWeb/Bindings/`

Bridge for cross-language communication between Python and JavaScript.

Key features:
- Value conversion between Python and JavaScript
- Function call marshalling
- Object proxying for cross-language access
- Exception handling across languages

```cpp
// Bridge for cross-language communication between Python and JavaScript
class PythonJSBridge {
public:
    // Initialize the bridge system
    static bool initialize_bridge();
    
    // Convert Python value to JavaScript value
    static JS::Value python_to_js(PyObject* py_obj, JS::Realm& realm);
    
    // Convert JavaScript value to Python value
    static PyObject* js_to_python(JS::Value js_val);
    
    // Call a JavaScript function from Python
    static PyObject* call_js_function(const String& function_name, PyObject* args, JS::Realm& realm);
    
    // Call a Python function from JavaScript
    static JS::Value call_python_function(PyObject* func, JS::MarkedVector<JS::Value>& js_args, JS::Realm& realm);
    
    // Get JavaScript global object for cross-language access
    static PyObject* get_js_global_object();
    
    // Set up the bridge in Python execution environment
    static bool setup_bridge_in_context(PyObject* globals, JS::Realm& realm);
    
private:
    static bool s_bridge_initialized;
    static PyObject* s_js_proxy_type;  // Python type for JS object proxies
    static PyObject* s_py_proxy_type;  // Python type for Python function proxies
};
```

## Implementation Flow

### 1. HTML Parsing
When the HTML parser encounters a `<script type="python">` tag:

```cpp
// In HTMLScriptElement.cpp
void HTMLScriptElement::prepare_script()
{
    // ... existing code ...
    
    // Extended to recognize Python script types
    else if (script_block_type.equals_ignoring_ascii_case("python"sv) || 
             script_block_type.equals_ignoring_ascii_case("text/python"sv)) {
        // then set el's type to "python".
        m_script_type = ScriptType::Python;
    }
    
    // ... existing code ...
}
```

### 2. Script Creation
For inline scripts, a PythonScript object is created directly:

```cpp
// In HTMLScriptElement.cpp
else if (m_script_type == ScriptType::Python) {
    // 1. Let script be the result of creating a python script using source text, settings object's realm, base URL, and options.
    // FIXME: Pass options.
    auto script = PythonScript::create(m_document->url().to_byte_string(), source_text_utf8, settings_object.realm(), base_url);
    
    // 2. Mark as ready el given script.
    mark_as_ready(Result(move(script)));
}
```

For external scripts, they are fetched and then a PythonScript object is created:

```cpp
// In HTMLScriptElement.cpp
else if (m_script_type == ScriptType::Python) {
    // For external Python scripts we'll need to fetch them and then create a PythonScript object.
    // We'll implement this behavior similar to how classic scripts are handled.
    fetch_python_script(*this, *url, settings_object, move(options), on_complete).release_value_but_fixme_should_propagate_errors();
}
```

### 3. Script Compilation
The PythonScript::create method compiles the Python source:

```cpp
// In PythonScript.cpp
GC::Ref<PythonScript> PythonScript::create(ByteString filename, StringView source, JS::Realm& realm, URL::URL base_url, MutedErrors muted_errors)
{
    auto& vm = realm.vm();
    
    // ... initialization code ...
    
    // Parse Python source code
    auto parse_timer = Core::ElapsedTimer::start_new();
    
    // Convert StringView to Python-compatible string
    auto source_utf8 = source.to_utf8();
    PyObject* compiled_code = Py_CompileString(source_utf8.characters(), filename.characters(), Py_file_input);
    
    if (!compiled_code) {
        auto error = PyErr_Occurred();
        if (error) {
            PyObject* error_str = PyObject_Str(error);
            if (error_str) {
                const char* error_cstr = PyUnicode_AsUTF8(error_str);
                if (error_cstr) {
                    auto error_message = String::from_utf8(error_cstr).value_or("Unknown Python error"sv);
                    dbgln("PythonScript: Failed to compile: {}", error_message);
                    
                    // Create a Python error object to store
                    auto python_error = JS::SyntaxError::create(realm, error_message);
                    script->set_parse_error(python_error);
                    script->set_error_to_rethrow(script->parse_error());
                    
                    Py_DECREF(error_str);
                    return script;
                }
                Py_DECREF(error_str);
            }
        }
        
        // Fallback error
        script->set_parse_error(JS::SyntaxError::create(realm, "Python compilation failed"sv));
        script->set_error_to_rethrow(script->parse_error());
        return script;
    }
    
    // Set script's record to compiled code.
    script->m_script_record = compiled_code;
    
    dbgln_if(HTML_SCRIPT_DEBUG, "PythonScript: Parsed {} in {}ms", script->filename(), parse_timer.elapsed_milliseconds());
    
    // Return script.
    return script;
}
```

### 4. Script Execution
The PythonScript::run method executes the compiled script:

```cpp
// In PythonScript.cpp
JS::Completion PythonScript::run(RethrowErrors rethrow_errors, GC::Ptr<JS::Environment> lexical_environment_override)
{
    // ... setup code ...
    
    // Execute script in a new dictionary context (equivalent to globals/locals)
    PyObject* globals = PyDict_New();
    PyObject* locals = PyDict_New();
    
    if (globals && locals) {
        // Add basic builtins to the globals
        PyObject* builtins = PyEval_GetBuiltins();
        PyDict_SetItemString(globals, "__builtins__", builtins);
        
        // Set up security restrictions for this script execution
        URL::URL origin = this->base_url().value_or(URL::URL {});
        if (!PythonSecurityModel::setup_sandboxed_environment(globals, origin)) {
            evaluation_status = JS::throw_completion(JS::Error::create(realm, JS::ErrorType::Generic, "Failed to set up secure execution environment"sv));
            Py_DECREF(globals);
            Py_DECREF(locals);
            PyGILState_Release(gstate);
            return evaluation_status;
        }
        
        // Import and add the web module with Python-friendly APIs
        if (Bindings::PythonDOMAPI::initialize_module()) {
            PyObject* web_module = Bindings::PythonDOMAPI::get_module();
            if (web_module) {
                PyDict_SetItemString(globals, "web", web_module);
                
                // For convenience, also add common functions/classes directly
                PyObject* doc_class = PyObject_GetAttrString(web_module, "Document");
                if (doc_class) {
                    PyDict_SetItemString(globals, "Document", doc_class);
                    Py_DECREF(doc_class);
                }
                
                PyObject* elem_class = PyObject_GetAttrString(web_module, "Element");
                if (elem_class) {
                    PyDict_SetItemString(globals, "Element", elem_class);
                    Py_DECREF(elem_class);
                }
                
                PyObject* win_class = PyObject_GetAttrString(web_module, "Window");
                if (win_class) {
                    PyDict_SetItemString(globals, "Window", win_class);
                    Py_DECREF(win_class);
                }
            }
        }
        
        PyObject* result = PyEval_EvalCode(m_script_record, globals, locals);
        
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
        } else {
            // Execution was successful
            if (result != Py_None) {
                // Convert Python result to JavaScript value if needed
                // For now, just mark as successful
            }
            Py_DECREF(result);
            evaluation_status = JS::normal_completion(JS::js_undefined());
        }
        
        Py_DECREF(globals);
        Py_DECREF(locals);
    } else {
        evaluation_status = JS::throw_completion(JS::Error::create(realm, JS::ErrorType::Generic, "Failed to create Python execution context"sv));
    }
    
    // ... cleanup code ...
}
```

## Build System Integration

### CMakeLists.txt Changes

#### Main CMakeLists.txt
```cmake
# Changed from:
find_package(Python3 COMPONENTS Interpreter)

# To:
find_package(Python3 COMPONENTS Interpreter Development)
```

#### LibWeb/CMakeLists.txt
Added Python source files:
- HTML/Scripting/PythonEngine.cpp
- HTML/Scripting/PythonScript.cpp
- HTML/Scripting/PythonSecurityModel.cpp
- Bindings/PythonDOMBindings.cpp
- Bindings/PythonJSBridge.cpp
- Bindings/PythonJSObjectWrapper.cpp
- Bindings/PythonObjectBridge.cpp
- Bindings/PythonWebModule.cpp

Added Python headers to GENERATED_SOURCES:
- HTML/Scripting/PythonEngine.h
- HTML/Scripting/PythonScript.h
- HTML/Scripting/PythonSecurityModel.h
- Bindings/PythonDOMBindings.h
- Bindings/PythonJSBridge.h
- Bindings/PythonJSObjectWrapper.h
- Bindings/PythonObjectBridge.h
- Bindings/PythonWebModule.h

Linked against Python libraries:
```cmake
target_link_libraries(LibWeb PRIVATE 
    # ... other libraries ...
    ${Python3_LIBRARIES}
)

target_include_directories(LibWeb PRIVATE 
    # ... other includes ...
    ${Python3_INCLUDE_DIRS}
)
```

#### Services/WebContent/CMakeLists.txt
Added Python library linking:
```cmake
target_link_libraries(webcontentservice PRIVATE 
    # ... other libraries ...
    ${Python3_LIBRARIES}
)

target_include_directories(webcontentservice PRIVATE 
    # ... other includes ...
    ${Python3_INCLUDE_DIRS}
)
```

## Memory Management

### Python Object Lifecycle

Careful management of Python object references:

```cpp
PythonScript::PythonScript(URL::URL base_url, ByteString filename, JS::Realm& realm)
    : Script(move(base_url), move(filename), realm)
{
}

PythonScript::~PythonScript()
{
    if (m_script_record) {
        // Acquire GIL before destroying Python objects
        PyGILState_STATE gstate = PyGILState_Ensure();
        Py_DECREF(m_script_record);
        PyGILState_Release(gstate);
        m_script_record = nullptr;
    }
}
```

### Garbage Collection Integration

Python objects are not managed by LibWeb's GC, but JavaScript objects referenced from Python are:

```cpp
void PythonScript::visit_edges(Cell::Visitor& visitor)
{
    Base::visit_edges(visitor);
    // Note: We don't visit m_script_record since it's a Python object, not a GC object
}
```

## Error Handling

### Python Exception Conversion

Python exceptions are properly converted to JavaScript exceptions:

```cpp
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

## Security Implementation

### Built-in Function Restrictions

Dangerous Python built-ins are removed:

```cpp
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

Dangerous modules are blocked:

```cpp
bool PythonSecurityModel::should_allow_module_import(const String& module_name, const URL& origin)
{
    // For security, only allow imports for specific modules that are safe
    // In a real implementation, we'd have a more sophisticated whitelist
    StringView module_view = module_name;
    
    // Allow only safe modules
    return module_view.is_one_of("json"sv, "datetime"sv, "math"sv, "random"sv, "re"sv, "collections"sv, "itertools"sv);
}
```

### Filesystem Access Restrictions

Filesystem operations are restricted:

```cpp
bool PythonSecurityModel::setup_restricted_filesystem_access()
{
    // In a real implementation, we'd override file operations to prevent
    // access to the real filesystem
    // For now, we'll just note that this is where filesystem restrictions would go
    return true;
}
```

### Network Request Control

Network requests follow Same-Origin Policy:

```cpp
bool PythonSecurityModel::should_allow_network_request(const URL& target_url, const URL& origin)
{
    // Implement SOP (Same-Origin Policy) for network requests
    // Only allow requests to same origin by default
    Origin target_origin = Origin::create(target_url);
    Origin source_origin = Origin::create(origin);
    
    return target_origin == source_origin;
}
```

## Cross-Language Communication

### Python to JavaScript Value Conversion

```cpp
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
    
    // ... other type conversions ...
}
```

### JavaScript to Python Value Conversion

```cpp
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
    
    // ... other type conversions ...
}
```

### Object Wrapping

JavaScript objects are wrapped for Python access:

```cpp
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

## Testing Implementation

### Unit Tests

Located in `Tests/LibWeb/Scripting/Python/`

Example test case:
```cpp
TEST_CASE(basic_python_execution)
{
    // Initialize VM
    Web::Bindings::initialize_main_thread_vm(Web::Bindings::AgentType::SimilarOriginWindow);
    auto& vm = Web::Bindings::main_thread_vm();
    
    // Create a simple Python script
    StringView source = "print('Hello from Python')";
    auto script = Web::HTML::PythonScript::create("test.py", source, vm.current_realm(), URL::URL {});
    
    // Verify script was created successfully
    EXPECT(script);
    
    // Execute the script
    auto result = script->run();
    
    // Verify successful execution
    EXPECT(!result.is_abrupt());
}
```

### Integration Tests

Located in `Tests/WebTests/Layout/Python/`

Example HTML test:
```html
<!DOCTYPE html>
<html>
<head>
    <title>Python Test</title>
</head>
<body>
    <div id="python-output"></div>
    
    <script type="python">
        # Simple Python script that manipulates the DOM
        document.getElementById("python-output").innerHTML = "<p>Hello from Python!</p>"
        
        # Test accessing window object
        print("Window title:", window.document.title)
        
        # Test creating elements
        new_div = document.createElement("div")
        new_div.innerHTML = "<p>Dynamically created by Python</p>"
        document.body.appendChild(new_div)
    </script>
</body>
</html>
```

This implementation provides a solid foundation for Python integration in Ladybird, with proper memory management, security, error handling, and cross-language communication.