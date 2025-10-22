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
    static GC::Ref<PythonScript> create(ByteString filename, StringView source, JS::Realm&, URL::URL base_url, MutedErrors = MutedErrors::No);
    JS::Completion run(RethrowErrors = RethrowErrors::No, GC::Ptr<JS::Environment> lexical_environment_override = {});
    
private:
    PyObject* m_script_record { nullptr };  // Compiled Python bytecode
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
    static bool is_initialized();
    static void* get_main_interpreter_state();
    static void* create_subinterpreter();
    static void destroy_subinterpreter(void* subinterpreter);
};
```

### PythonSecurityModel.h/.cpp
Located in `Libraries/LibWeb/HTML/Scripting/`

Implements sandboxing for Python script execution.

Key features:
- Restricts dangerous Python built-ins
- Blocks access to dangerous modules
- Implements Same-Origin Policy
- Controls filesystem and network access

### PythonDOMBindings.h/.cpp
Located in `Libraries/LibWeb/Bindings/`

Provides Python-friendly bindings to DOM APIs.

Key components:
- PythonDocument wrapper for DOM Document
- PythonElement wrapper for DOM Elements
- PythonWindow wrapper for Window object
- Module initialization functions

```cpp
class PythonDOMAPI {
public:
    static bool initialize_module();
    static PyObject* get_module();
};
```

### MainThreadVM.cpp Modifications
Located in `Libraries/LibWeb/Bindings/`

Initializes the Python engine alongside the JavaScript VM.

Key changes:
- Added `HTML::PythonEngine::initialize()` call during VM initialization
- Added cleanup handler for proper Python engine shutdown

## Build System Changes

### CMakeLists.txt Modifications

1. **LibWeb/CMakeLists.txt**
   - Added Python source files to the build
   - Added Python headers to the header list
   - Linked against Python3 libraries

2. **Services/WebContent/CMakeLists.txt**
   - Added Python3_LIBRARIES to target_link_libraries
   - Added Python3_INCLUDE_DIRS to target_include_directories

3. **Main CMakeLists.txt**
   - Changed from `find_package(Python3 COMPONENTS Interpreter)` to `find_package(Python3 COMPONENTS Interpreter Development)`

## HTML Parser Integration

### HTMLScriptElement.cpp Changes

Modified the script type determination logic to recognize Python:

```cpp
// Extended script type detection to include Python
else if (script_block_type.equals_ignoring_ascii_case("python"sv) || 
         script_block_type.equals_ignoring_ascii_case("text/python"sv)) {
    m_script_type = ScriptType::Python;
}
```

Modified script execution to handle Python scripts:

```cpp
// Case for Python script execution
else if (m_script_type == ScriptType::Python) {
    // Execute Python script
    (void)as<PythonScript>(*m_result.get<GC::Ref<Script>>()).run();
}
```

## Key Implementation Patterns

### Python Object Management

All Python objects are managed with proper reference counting:

```cpp
// Example of proper Python object management
PyObject* obj = PyDict_New();
if (obj) {
    // Use the object
    // ...
    
    // Release the reference
    Py_DECREF(obj);
}
```

### GIL Management

The Global Interpreter Lock is properly acquired and released:

```cpp
// Proper GIL acquisition
PyGILState_STATE gstate = PyGILState_Ensure();

// Python operations here
// ...

// Proper GIL release
PyGILState_Release(gstate);
```

### Error Handling

Python errors are properly caught and converted to JavaScript exceptions:

```cpp
PyObject* result = PyEval_EvalCode(m_script_record, globals, locals);

if (!result) {
    // Python error occurred
    PyObject* error_type = nullptr;
    PyObject* error_value = nullptr;
    PyObject* error_traceback = nullptr;
    
    PyErr_Fetch(&error_type, &error_value, &error_traceback);
    PyErr_NormalizeException(&error_type, &error_value, &error_traceback);
    
    // Convert to JavaScript exception
    // ...
}
```

## Cross-Language Bridge

The implementation provides bidirectional communication between Python and JavaScript:

### Python to JavaScript
- Python values are converted to JavaScript values using `python_to_js()`
- Python objects can access JavaScript objects through wrappers

### JavaScript to Python
- JavaScript values are converted to Python values using `js_to_python()`
- JavaScript objects are wrapped in Python objects for access

## Testing

A test HTML file has been created to verify functionality:

```html
<!DOCTYPE html>
<html>
<head>
    <title>Python Test</title>
</head>
<body>
    <h1>Python Script Test</h1>
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

## Performance Considerations

1. **GIL Contention**: Python's GIL may become a bottleneck for concurrent script execution
2. **Object Conversion**: Converting objects between Python and JavaScript has overhead
3. **Subinterpreters**: Using subinterpreters for isolation can increase memory usage
4. **Compilation Caching**: Python bytecode compilation should be cached for repeated executions

## Security Model

The security model implements several layers of protection:

1. **Built-in Restrictions**: Dangerous Python built-ins are removed or restricted
2. **Module Restrictions**: Access to dangerous modules (os, sys, subprocess, etc.) is blocked
3. **Filesystem Restrictions**: Filesystem access is limited or prohibited
4. **Network Restrictions**: Network requests are subject to Same-Origin Policy
5. **Sandboxed Execution**: Scripts run in isolated environments when possible

## Future Improvements

1. **Performance Optimization**: Reduce overhead in cross-language conversions
2. **Enhanced Security**: More sophisticated sandboxing mechanisms
3. **Better Error Reporting**: Richer error information with stack traces
4. **Module System**: Standard library module access with security controls
5. **Async Support**: Better integration with browser's async event model
6. **Debugging Tools**: Developer tools for debugging Python scripts