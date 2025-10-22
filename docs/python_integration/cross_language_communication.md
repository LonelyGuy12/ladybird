# Cross-Language Communication Between Python and JavaScript

## Overview

The cross-language communication system enables seamless interaction between Python and JavaScript code within the browser environment. This allows developers to mix both languages in their web applications while maintaining consistent access to DOM APIs and browser functionality.

## Communication Patterns

### 1. Global Object Sharing

Both languages share access to the same global objects:

```python
# Python accessing JavaScript objects
js_window = window
js_document = document

# Python modifying JavaScript objects
window.py_property = "Hello from Python"
```

```javascript
// JavaScript accessing Python objects
py_window = window;
py_document = document;

// JavaScript modifying Python objects
window.jsProperty = "Hello from JavaScript";
```

### 2. Function Calling Across Languages

Functions can be called from one language to another:

```python
# Define a Python function
def python_function(arg1, arg2):
    return arg1 + arg2

# Make it accessible to JavaScript
window.python_func = python_function
```

```javascript
// Call Python function from JavaScript
var result = window.python_func(10, 20);
console.log(result); // Outputs: 30
```

### 3. Event Handling

Events can be handled by functions written in either language:

```python
# Python event handler
def python_click_handler(event):
    print("Clicked from Python!")
    event.prevent_default()
    
# Register handler
button.add_event_listener('click', python_click_handler)
```

```javascript
// JavaScript event handler
function jsClickHandler(event) {
    console.log("Clicked from JavaScript!");
    event.preventDefault();
}

// Register handler
button.addEventListener('click', jsClickHandler);
```

## Implementation Architecture

### PythonJSBridge

The core bridge component that handles:
- Value conversion between Python and JavaScript
- Function call marshalling
- Object proxy creation
- Exception handling

### PythonJSObjectWrapper

Wrapper class that creates Python proxies for JavaScript objects:
- Implements Python object protocol (`__getattr__`, `__setattr__`, etc.)
- Forwards method calls to JavaScript objects
- Handles property access and modification
- Manages reference counting

### JSObjectWrapper (Python)

Python wrapper for JavaScript objects:
- Inherits from `PyObject_HEAD`
- Contains pointer to underlying JavaScript object
- Implements Python methods that delegate to JavaScript
- Handles proper memory management

## Value Conversion

### Python to JavaScript Conversion

| Python Type | JavaScript Type | Notes |
|-------------|-----------------|-------|
| `str` | `String` | UTF-8 conversion |
| `int` | `Number` | 32-bit integers |
| `float` | `Number` | Double precision |
| `bool` | `Boolean` | True/False |
| `None` | `undefined` | |
| `list`/`tuple` | `Array` | Recursive conversion |
| `dict` | `Object` | String keys only |
| Custom objects | Proxy objects | Method forwarding |

### JavaScript to Python Conversion

| JavaScript Type | Python Type | Notes |
|-----------------|-------------|-------|
| `String` | `str` | UTF-8 conversion |
| `Number` | `int`/`float` | Automatic type detection |
| `Boolean` | `bool` | True/False |
| `undefined`/`null` | `None` | |
| `Array` | `list` | Recursive conversion |
| `Object` | `dict` or custom wrapper | Depends on object type |
| Functions | Callable proxy | Method forwarding |
| Custom objects | Proxy objects | Property access |

## Object Proxying

### JavaScript Objects in Python

JavaScript objects accessed from Python are wrapped in proxy objects:

```python
# Access JavaScript object from Python
js_obj = window.some_js_object

# Property access
value = js_obj.some_property

# Method calls
result = js_obj.some_method(arg1, arg2)

# Property modification
js_obj.new_property = "Python value"
```

Proxy implementation details:
- Uses `__getattr__` to forward property access to JavaScript
- Uses `__setattr__` to forward property modification to JavaScript
- Uses `__call__` to make objects callable if they're functions
- Maintains proper reference counting to prevent garbage collection

### Python Objects in JavaScript

Python objects accessible from JavaScript are wrapped in proxy objects:

```javascript
// Access Python object from JavaScript
pyObj = window.some_python_object;

// Property access
var value = pyObj.someProperty;

// Method calls
var result = pyObj.someMethod(arg1, arg2);

// Property modification
pyObj.newProperty = "JavaScript value";
```

Proxy implementation details:
- Uses JavaScript Proxy objects to intercept property access
- Forwards calls to Python object methods
- Handles conversion of arguments and return values
- Maintains proper reference counting

## Function Call Marshalling

### Calling JavaScript Functions from Python

```python
# Define Python function to call JavaScript
def call_js_function(js_func, *args):
    # Convert Python arguments to JavaScript values
    js_args = [python_to_js(arg) for arg in args]
    
    # Call JavaScript function
    result = js_func(*js_args)
    
    # Convert result back to Python
    return js_to_python(result)
```

### Calling Python Functions from JavaScript

```javascript
// Define JavaScript function to call Python
function callPythonFunction(pyFunc, ...args) {
    // Convert JavaScript arguments to Python values
    const pyArgs = args.map(arg => jsToPython(arg));
    
    // Call Python function
    const result = pyFunc(...pyArgs);
    
    // Convert result back to JavaScript
    return pythonToJs(result);
}
```

## Exception Handling

Exceptions are properly propagated across language boundaries:

### Python Exceptions in JavaScript

```python
# Python function that raises an exception
def python_function():
    raise ValueError("Something went wrong in Python")
```

```javascript
// JavaScript code that catches Python exception
try {
    window.python_function();
} catch (e) {
    console.log("Caught Python exception:", e.message);
}
```

### JavaScript Exceptions in Python

```javascript
// JavaScript function that throws an exception
function jsFunction() {
    throw new Error("Something went wrong in JavaScript");
}
```

```python
# Python code that catches JavaScript exception
try:
    window.js_function()
except Exception as e:
    print(f"Caught JavaScript exception: {e}")
```

## Memory Management

Cross-language objects require careful memory management:

### Reference Counting

- Python objects use reference counting (`Py_INCREF`/`Py_DECREF`)
- JavaScript objects use garbage collection
- Proxy objects maintain references to both sides
- Proper cleanup prevents memory leaks

### Garbage Collection Integration

- Python objects referenced from JavaScript are protected from GC
- JavaScript objects referenced from Python are protected from GC
- Circular references are detected and broken appropriately
- Cleanup occurs when objects are no longer accessible

## Performance Considerations

### Conversion Overhead

Each cross-language access involves:
1. Type checking and conversion
2. Object wrapping/unwrapping
3. Language boundary crossing
4. Reference counting adjustments

### Optimization Strategies

1. **Caching**: Cache frequently accessed properties
2. **Batching**: Batch multiple operations when possible
3. **Reduced Crossing**: Minimize language boundary crossings
4. **Direct Access**: Use direct C++ access when available

## Security Implications

Cross-language communication has security considerations:

### Sandboxing

- Both languages operate within their respective sandboxes
- Cross-language calls respect security boundaries
- Same-Origin Policy applies to all operations
- Dangerous operations are blocked in both languages

### Data Validation

- All data crossing language boundaries is validated
- Malformed data is rejected appropriately
- Type conversions are safe and predictable
- Buffer overflows and other vulnerabilities are prevented

## Testing Cross-Language Features

### Unit Tests

```python
# Test Python calling JavaScript
def test_python_calls_js():
    # Create JavaScript function
    window.js_func = lambda x: x * 2
    
    # Call from Python
    result = window.js_func(21)
    
    # Verify result
    assert result == 42
```

```javascript
// Test JavaScript calling Python
function testJsCallsPython() {
    // Create Python function
    window.pyFunc = (x) => x * 2;
    
    // Call from JavaScript
    const result = window.pyFunc(21);
    
    // Verify result
    console.assert(result === 42);
}
```

### Integration Tests

Complex scenarios involving both languages:
- Event handling with mixed language handlers
- DOM manipulation from both languages
- Asynchronous operations spanning languages
- Error propagation across language boundaries

## Future Improvements

### 1. Performance Enhancements

- Optimize conversion routines for common types
- Implement fast paths for simple operations
- Reduce object wrapping overhead
- Improve cache efficiency

### 2. Extended API Coverage

- More complete DOM API exposure
- Web API integration (Fetch, WebSockets, etc.)
- Better support for complex JavaScript objects
- Enhanced error handling and reporting

### 3. Debugging Support

- Cross-language stack traces
- Debugging tools for mixed-language applications
- Profiling for cross-language performance
- Memory leak detection

### 4. Advanced Features

- Generator/iterator support across languages
- Promise/async integration
- Better module system interoperability
- Enhanced type mapping for complex objects