# Python Integration Architecture

## Overview

The Python integration in Ladybird follows the same architectural patterns as the existing JavaScript implementation, with a layered approach that separates concerns while maintaining tight integration with the browser's core systems.

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    HTML Document Layer                      │
├─────────────────────────────────────────────────────────────┤
│                 HTMLScriptElement Parser                   │
│  ┌─────────────┐  ┌─────────────┐  ┌────────────────────┐ │
│  │ JavaScript  │  │  Python     │  │ Other Script Types │ │
│  │   Script    │  │   Script    │  │                    │ │
│  └─────────────┘  └─────────────┘  └────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│               Script Management Layer                       │
│  ┌─────────────────┐  ┌──────────────────┐               │
│  │ ClassicScript   │  │ PythonScript     │               │
│  │ (JavaScript)    │  │                  │               │
│  └─────────────────┘  └──────────────────┘               │
├─────────────────────────────────────────────────────────────┤
│                Execution Engine Layer                       │
│  ┌─────────────────┐  ┌──────────────────┐               │
│  │    JS::VM       │  │  PythonEngine    │               │
│  │ (LibJS)         │  │ (CPython)        │               │
│  └─────────────────┘  └──────────────────┘               │
├─────────────────────────────────────────────────────────────┤
│                  DOM Binding Layer                          │
│  ┌─────────────────┐  ┌──────────────────┐               │
│  │ JS Bindings     │  │ PythonDOMAPI     │               │
│  │ (LibWeb)        │  │ (Python Bindings)│               │
│  └─────────────────┘  └──────────────────┘               │
├─────────────────────────────────────────────────────────────┤
│                 Security Model Layer                        │
│  ┌─────────────────┐  ┌──────────────────┐               │
│  │ JSSecurityModel │  │PythonSecurityModel│               │
│  │ (JavaScript)    │  │ (Python)         │               │
│  └─────────────────┘  └──────────────────┘               │
├─────────────────────────────────────────────────────────────┤
│              Cross-Language Bridge Layer                    │
│  ┌────────────────────────────────────────────────────────┐ │
│  │           PythonJSBridge & JSObjectWrapper           │ │
│  │                                                        │ │
│  │  ┌──────────────┐    ┌─────────────────┐             │ │
│  │  │ Python Values│◄──►│ JavaScript Values│             │ │
│  │  └──────────────┘    └─────────────────┘             │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## Component Details

### 1. HTML Document Layer

#### HTMLScriptElement Parser Modifications
The HTML parser has been extended to recognize Python script types:

```cpp
// In HTMLScriptElement.cpp
void HTMLScriptElement::prepare_script()
{
    // ... existing code ...
    
    // Extended to recognize Python script types
    else if (script_block_type.equals_ignoring_ascii_case("python"sv) || 
             script_block_type.equals_ignoring_ascii_case("text/python"sv)) {
        m_script_type = ScriptType::Python;
    }
    
    // ... existing code ...
}
```

This allows HTML documents to include Python scripts:
```html
<script type="python">
    # Python code goes here
    document.find("#output").text = "Hello from Python!"
</script>
```

### 2. Script Management Layer

#### PythonScript Class
The `PythonScript` class parallels the `ClassicScript` class for JavaScript:

```cpp
class PythonScript final : public Script {
    GC_CELL(PythonScript, Script);
    GC_DECLARE_ALLOCATOR(PythonScript);

public:
    static GC::Ref<PythonScript> create(ByteString filename, StringView source, JS::Realm&, URL::URL base_url, MutedErrors = MutedErrors::No);
    
    JS::Completion run(RethrowErrors = RethrowErrors::No, GC::Ptr<JS::Environment> lexical_environment_override = {});
    
    PyObject* script_record() { return m_script_record; }
    PyObject const* script_record() const { return m_script_record; }

private:
    PythonScript(URL::URL base_url, ByteString filename, JS::Realm&);
    
    virtual bool is_python_script() const final { return true; }
    virtual void visit_edges(Cell::Visitor&) override;
    
    PyObject* m_script_record { nullptr };
    MutedErrors m_muted_errors { MutedErrors::No };
};
```

Key responsibilities:
- Parse Python source code into bytecode using `Py_CompileString`
- Manage compiled script records
- Execute scripts with proper error handling
- Interface with the security model

### 3. Execution Engine Layer

#### PythonEngine Class
The `PythonEngine` manages the CPython interpreter lifecycle:

```cpp
class PythonEngine {
public:
    static void initialize();
    static void shutdown();
    static bool is_initialized();
    static void* get_main_interpreter_state();
    static void* create_subinterpreter();
    static void destroy_subinterpreter(void* subinterpreter);
    
private:
    static bool s_initialized;
};
```

Key features:
- Initialize Python interpreter with `Py_Initialize()`
- Manage Global Interpreter Lock (GIL) with `PyGILState_Ensure()/Release()`
- Provide subinterpreter support for script isolation
- Handle proper shutdown with `Py_Finalize()`

Implementation details:
```cpp
void PythonEngine::initialize()
{
    if (s_initialized) {
        return;
    }
    
    // Initialize Python interpreter
    Py_Initialize();
    
    // Initialize GIL
    PyEval_InitThreads();
    
    // Release GIL since we'll acquire it as needed
    PyThreadState* mainThreadState = PyEval_SaveThread();
    
    // Initialize security model
    PythonSecurityModel::initialize_security();
    
    s_initialized = true;
}

void PythonEngine::shutdown()
{
    if (!s_initialized) {
        return;
    }
    
    PyGILState_Ensure();  // Acquire GIL before finalizing
    Py_Finalize();
    s_initialized = false;
}
```

### 4. DOM Binding Layer

#### PythonDOMAPI Module
Provides Python-idiomatic bindings to DOM APIs:

```cpp
class PythonDOMAPI {
public:
    static bool initialize_module();
    static PyObject* get_module();
    
private:
    static PyObject* s_module;
    static bool s_initialized;
};
```

Components:
- PythonDocument wrapper for DOM Document
- PythonElement wrapper for DOM Elements
- PythonWindow wrapper for Window object
- Helper functions for cross-language conversion

#### Python-Friendly APIs

The Python DOM API provides familiar syntax:

```python
# CSS selector queries
elements = document.select('.my-class')  # Returns list
element = document.find('#my-id')       # Returns single element or None

# Property access
text = element.text
html = element.html

# Method calls
element.set_attribute('class', 'new-class')
value = element.get_attribute('data-value')
```

### 5. Security Model Layer

#### PythonSecurityModel Class
Implements sandboxing for Python script execution:

```cpp
class PythonSecurityModel {
public:
    static bool initialize_security();
    static bool should_allow_script_execution(const String& script_content, const URL& origin);
    static bool setup_sandboxed_environment(PyObject* globals, const URL& origin);
    static bool restrict_builtins(PyObject* globals);
    static bool should_allow_module_import(const String& module_name, const URL& origin);
    static bool setup_restricted_filesystem_access();
    static bool should_allow_network_request(const URL& target_url, const URL& origin);
    
private:
    static bool s_security_initialized;
};
```

Key security features:
- Built-in function restrictions
- Module import controls
- Filesystem access limitations
- Same-Origin Policy enforcement
- Network request filtering

### 6. Cross-Language Bridge Layer

#### PythonJSBridge Class
Enables communication between Python and JavaScript:

```cpp
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
    static PyObject* s_js_proxy_type;
    static PyObject* s_py_proxy_type;
};
```

#### Value Conversion

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

#### Object Wrapping

Cross-language object access through proxy wrappers:

```cpp
// JavaScript object wrapper for Python
class PythonJSObjectWrapper {
public:
    static PyObject* create_wrapper(JS::Object& js_obj);
    static JS::Object* get_cpp_object(PyObject* obj);
    static void setup_type();
    
private:
    static PyTypeObject s_type;
};

// Python object wrapper for JavaScript
class PythonObjectBridge {
public:
    static JS::Object* create_wrapper(PyObject* py_obj, JS::Realm& realm);
    static PyObject* get_python_object(JS::Object* obj);
    static void setup_type();
    
private:
    static JS::Object* s_type;
};
```

## Integration Points

### 1. Main Thread VM Initialization

The Python engine is initialized alongside the JavaScript VM:

```cpp
// In MainThreadVM.cpp
void initialize_main_thread_vm(AgentType type)
{
    VERIFY(!s_main_thread_vm);

    // Initialize Python engine before creating the JS VM
    HTML::PythonEngine::initialize();

    s_main_thread_vm = JS::VM::create();
    // ... rest of initialization
}
```

### 2. HTML Script Processing

Python scripts are processed similarly to JavaScript:

```cpp
// In HTMLScriptElement.cpp
void HTMLScriptElement::execute_script()
{
    // ... setup code ...
    
    // Switch on script type
    if (m_script_type == ScriptType::Classic) {
        // JavaScript execution
        (void)as<ClassicScript>(*m_result.get<GC::Ref<Script>>()).run();
    }
    else if (m_script_type == ScriptType::Python) {
        // Python execution
        (void)as<PythonScript>(*m_result.get<GC::Ref<Script>>()).run();
    }
    // ... other script types ...
}
```

### 3. Resource Loading

External Python scripts are fetched and compiled:

```cpp
// Fetch external Python script
WebIDL::ExceptionOr<void> fetch_python_script(
    GC::Ref<HTMLScriptElement> element, 
    URL::URL const& url, 
    EnvironmentSettingsObject& settings_object,
    ScriptFetchOptions options, 
    OnFetchScriptComplete on_complete)
{
    // Create request for Python script
    auto request = create_potential_CORS_request(
        vm, url, Fetch::Infrastructure::Request::Destination::Script, 
        CORSSettingAttribute::NoCORS);
    
    // Set up request
    request->set_client(&settings_object);
    request->set_initiator_type(Fetch::Infrastructure::Request::InitiatorType::Script);
    
    // Set up Python-specific request options
    set_up_classic_script_request(*request, options);
    
    // Fetch and process response
    Fetch::Infrastructure::FetchAlgorithms::Input fetch_algorithms_input {};
    fetch_algorithms_input.process_response_consume_body = 
        [&settings_object, options = move(options), on_complete = move(on_complete)](
            auto response, auto body_bytes) {
        
        // Handle response
        if (body_bytes.template has<Empty>() || 
            body_bytes.template has<Fetch::Infrastructure::FetchAlgorithms::ConsumeBodyFailureTag>() || 
            !Fetch::Infrastructure::is_ok_status(response->status())) {
            on_complete->function()(nullptr);
            return;
        }
        
        // Extract content
        auto body_buffer = body_bytes.template get<ByteBuffer>();
        auto source_text = TextCodec::convert_input_to_utf8_using_given_decoder_unless_there_is_a_byte_order_mark(
            TextCodec::decoder_for("UTF-8"sv).value(), body_buffer).release_value_but_fixme_should_propagate_errors();
        
        // Create Python script
        auto script = PythonScript::create(
            response->url().value_or({}).to_byte_string(), 
            source_text, 
            settings_object.realm(), 
            response->url().value_or({}));
        
        // Complete fetch
        on_complete->function()(script);
    };
    
    TRY(Fetch::Fetching::fetch(element->realm(), request, 
        Fetch::Infrastructure::FetchAlgorithms::create(vm, move(fetch_algorithms_input))));
    
    return {};
}
```

## Memory Management

### Reference Counting

Cross-language objects require careful memory management:

1. **Python Objects**: Use `Py_INCREF`/`Py_DECREF` for reference counting
2. **JavaScript Objects**: Use GC references with proper visit_edges implementation
3. **Wrapper Objects**: Maintain references to both sides to prevent premature collection

### Garbage Collection Integration

Integration with both Python and JavaScript garbage collectors:

```cpp
// In PythonScript.cpp
void PythonScript::visit_edges(Cell::Visitor& visitor)
{
    Base::visit_edges(visitor);
    // Note: We don't visit m_script_record since it's a Python object, not a GC object
}
```

```cpp
// In PythonJSObjectWrapper.cpp
void PythonJSObjectWrapper::setup_type()
{
    if (s_type.tp_name) return; // Already set up
    
    static PyTypeObject type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "web.JSObject",
        .tp_doc = "JavaScript object wrapper for Python",
        .tp_basicsize = sizeof(PythonJSObjectWrapper),
        .tp_itemsize = 0,
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_dealloc = (destructor)python_js_object_wrapper_dealloc,
        .tp_getattro = (getattrofunc)python_js_object_wrapper_getattr,
        .tp_setattro = (setattrofunc)python_js_object_wrapper_setattr,
    };
    
    memcpy(&s_type, &type, sizeof(PyTypeObject));
    
    if (PyType_Ready(&s_type) < 0) {
        // Handle error appropriately
    }
}
```

## Threading Model

### Single-Threaded Execution

Python scripts execute in the WebContent process on the main thread:

1. **GIL Management**: Properly acquire and release Python's Global Interpreter Lock
2. **Event Loop Integration**: Ensure Python execution doesn't block the browser UI
3. **Subinterpreter Isolation**: Use subinterpreters for script isolation when needed

### Concurrency Considerations

For handling concurrent script execution:

```cpp
void* PythonEngine::create_subinterpreter()
{
    if (!s_initialized) {
        return nullptr;
    }
    
    PyGILState_STATE gstate = PyGILState_Ensure();
    
    // Create a new subinterpreter for isolation
    PyThreadState* subThreadState = Py_NewInterpreter();
    
    PyGILState_Release(gstate);
    
    return subThreadState;
}
```

## Error Handling

### Cross-Language Exception Propagation

Proper error handling between languages:

```cpp
// Convert Python exceptions to JavaScript
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
            evaluation_status = JS::throw_completion(
                JS::Error::create(realm, JS::ErrorType::Generic, error_message));
        }
        Py_DECREF(error_str);
    }
    Py_DECREF(error_value);
}
if (error_type) Py_DECREF(error_type);
if (error_traceback) Py_DECREF(error_traceback);
```

## Performance Considerations

### Optimization Strategies

1. **Caching**: Cache frequently accessed objects and methods
2. **Batching**: Batch operations to reduce cross-language boundary crossings
3. **Lazy Evaluation**: Defer expensive operations until needed
4. **Memory Pooling**: Reuse objects to reduce allocation overhead

### Profiling and Monitoring

Built-in performance monitoring:

```cpp
auto timer = Core::ElapsedTimer::start_new();

// Execute Python script
PyObject* result = PyEval_EvalCode(m_script_record, globals, locals);

dbgln_if(HTML_SCRIPT_DEBUG, "PythonScript: Finished running script {}, Duration: {}ms", 
         filename(), timer.elapsed_milliseconds());
```

## Security Architecture

### Multi-Layer Security Model

1. **Built-in Restrictions**: Remove dangerous Python functions
2. **Module Controls**: Whitelist safe modules only
3. **Filesystem Access**: Restrict or redirect file operations
4. **Network Policies**: Enforce Same-Origin Policy
5. **Resource Limits**: Prevent resource exhaustion

### Implementation Example

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
    
    // Allow only safe built-in functions
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

This architecture provides a solid foundation for Python integration while maintaining the security, performance, and compatibility requirements of a modern web browser.