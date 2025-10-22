# Testing Strategy for Python Integration

## Overview

A comprehensive testing strategy is essential to ensure the Python integration in Ladybird is reliable, secure, and performs well. This document outlines the testing approach for all aspects of the Python implementation.

## Test Categories

### 1. Unit Tests

Located in `Tests/LibWeb/Scripting/Python/`

#### Core Functionality Tests
Test basic Python script parsing and execution:

```cpp
// Tests/LibWeb/Scripting/Python/BasicExecutionTest.cpp
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

#### DOM API Tests
Test Python access to DOM APIs:

```cpp
// Tests/LibWeb/Scripting/Python/DOMAPITest.cpp
TEST_CASE(python_dom_api_access)
{
    // Initialize VM and create test document
    Web::Bindings::initialize_main_thread_vm(Web::Bindings::AgentType::SimilarOriginWindow);
    auto& vm = Web::Bindings::main_thread_vm();
    
    // Create test document
    auto document = Web::DOM::Document::create(vm.heap(), Web::DOM::Document::Type::HTML);
    
    // Create PythonDocument wrapper
    PyObject* py_doc = Web::Bindings::PythonDOMAPI::create_wrapper_for_dom_document(*document);
    EXPECT(py_doc);
    
    // Test property access
    PyObject* title_attr = PyObject_GetAttrString(py_doc, "title");
    EXPECT(title_attr);
    
    // Test method calls
    PyObject* args = PyTuple_New(1);
    PyTuple_SetItem(args, 0, PyUnicode_FromString("div"));
    PyObject* result = PyObject_CallMethod(py_doc, "create_element", "O", args);
    Py_DECREF(args);
    EXPECT(result);
    
    Py_DECREF(result);
    Py_DECREF(title_attr);
    Py_DECREF(py_doc);
}
```

#### Security Tests
Test security model restrictions:

```cpp
// Tests/LibWeb/Scripting/Python/SecurityTest.cpp
TEST_CASE(python_security_restrictions)
{
    // Initialize VM
    Web::Bindings::initialize_main_thread_vm(Web::Bindings::AgentType::SimilarOriginWindow);
    auto& vm = Web::Bindings::main_thread_vm();
    
    // Test dangerous module import restriction
    StringView source = "import os";
    auto script = Web::HTML::PythonScript::create("test.py", source, vm.current_realm(), URL::URL {});
    
    // Execute the script
    auto result = script->run();
    
    // Should fail due to security restrictions
    EXPECT(result.is_abrupt());
    
    // Test filesystem access restriction
    source = "f = open('/etc/passwd', 'r')";
    script = Web::HTML::PythonScript::create("test2.py", source, vm.current_realm(), URL::URL {});
    
    // Execute the script
    result = script->run();
    
    // Should fail due to security restrictions
    EXPECT(result.is_abrupt());
}
```

### 2. Integration Tests

Located in `Tests/LibWeb/HTML/Scripting/`

#### HTML Parser Integration Tests
Test that HTML parser correctly recognizes Python script tags:

```cpp
// Tests/LibWeb/HTML/Scripting/HTMLScriptElementTest.cpp
TEST_CASE(html_script_element_python_recognition)
{
    // Parse HTML with Python script tag
    StringView html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <script type="python">
                print("Hello from Python")
            </script>
        </head>
        <body></body>
        </html>
    )";
    
    // Parse the document
    auto document = parse_html_document(html);
    EXPECT(document);
    
    // Find the script element
    auto scripts = document->get_elements_by_tag_name("script");
    EXPECT_EQ(scripts->length(), 1u);
    
    auto script_element = scripts->item(0);
    EXPECT(script_element);
    
    // Verify it's recognized as a Python script
    auto& html_script = static_cast<Web::HTML::HTMLScriptElement&>(*script_element);
    EXPECT_EQ(html_script.script_type(), Web::HTML::HTMLScriptElement::ScriptType::Python);
}
```

#### Script Execution Integration Tests
Test that Python scripts execute correctly in the browser context:

```cpp
// Tests/LibWeb/HTML/Scripting/ScriptExecutionTest.cpp
TEST_CASE(python_script_execution_in_browser_context)
{
    // Create test HTML with Python script
    StringView html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <title>Test Document</title>
        </head>
        <body>
            <div id="output"></div>
            <script type="python">
                document.find("#output").text = "Python executed successfully!"
                window_title = window.document.title
                print(f"Window title: {window_title}")
            </script>
        </body>
        </html>
    )";
    
    // Parse and execute
    auto document = parse_and_execute_html(html);
    EXPECT(document);
    
    // Verify DOM manipulation worked
    auto output_element = document->get_element_by_id("output");
    EXPECT(output_element);
    EXPECT_EQ(output_element->text_content(), "Python executed successfully!"_string);
}
```

### 3. Layout Tests

Located in `Tests/WebTests/Layout/Python/`

#### Basic Rendering Tests
Test that Python DOM manipulation affects rendering:

```html
<!-- Tests/WebTests/Layout/Python/basic-rendering.html -->
<!DOCTYPE html>
<html>
<head>
    <title>Python Rendering Test</title>
</head>
<body>
    <div id="container"></div>
    
    <script type="python">
        # Create and append elements
        for i in range(5):
            div = document.create_element("div")
            div.text = f"Python div #{i}"
            div.set_attribute("class", "python-element")
            document.find("#container").append_child(div)
    </script>
</body>
</html>
```

Expected rendering output:
```
Python div #0
Python div #1
Python div #2
Python div #3
Python div #4
```

#### CSS Selector Tests
Test Python CSS selector functionality:

```html
<!-- Tests/WebTests/Layout/Python/css-selectors.html -->
<!DOCTYPE html>
<html>
<head>
    <title>CSS Selector Test</title>
</head>
<body>
    <div class="content">First content div</div>
    <div class="content highlight">Second content div</div>
    <div class="sidebar">Sidebar div</div>
    <p id="paragraph">Paragraph element</p>
    
    <script type="python">
        # Test select() method
        content_divs = document.select(".content")
        print(f"Found {len(content_divs)} content divs")
        
        # Test find() method
        highlighted = document.find(".highlight")
        if highlighted:
            print(f"Found highlighted element: {highlighted.text}")
        
        # Test ID selector
        paragraph = document.find("#paragraph")
        if paragraph:
            print(f"Found paragraph: {paragraph.text}")
    </script>
</body>
</html>
```

#### Event Handling Tests
Test Python event handling:

```html
<!-- Tests/WebTests/Layout/Python/event-handling.html -->
<!DOCTYPE html>
<html>
<head>
    <title>Event Handling Test</title>
</head>
<body>
    <button id="test-button">Click Me</button>
    <div id="output"></div>
    
    <script type="python">
        def button_clicked(event):
            output = document.find("#output")
            output.text = "Button clicked from Python!"
            print("Python event handler executed")
        
        button = document.find("#test-button")
        button.add_event_listener("click", button_clicked)
    </script>
</body>
</html>
```

### 4. Cross-Language Communication Tests

Located in `Tests/WebTests/Layout/Python/Interop/`

#### Python Calling JavaScript Tests
```html
<!-- Tests/WebTests/Layout/Python/Interop/python-calls-js.html -->
<!DOCTYPE html>
<html>
<head>
    <title>Python Calls JavaScript Test</title>
</head>
<body>
    <div id="js-output"></div>
    
    <script>
        // JavaScript function to be called from Python
        function jsFunction(message) {
            console.log("Called from Python:", message);
            return "Hello from JavaScript!";
        }
        
        // JavaScript object to be accessed from Python
        window.jsObject = {
            property: "JS property value",
            method: function() {
                return "JS method called";
            }
        };
    </script>
    
    <script type="python">
        # Python calling JavaScript
        result = window.jsFunction("Hello from Python")
        print(result)  # Should output: Hello from JavaScript!
        
        # Python accessing JavaScript object
        js_prop = window.jsObject.property
        js_method_result = window.jsObject.method()
        print(f"JS property: {js_prop}, JS method: {js_method_result}")
        
        # Python setting JavaScript values
        window.pyProperty = "Python property value"
        window.pyFunction = lambda x: f"Python function called with {x}"
    </script>
    
    <script>
        // JavaScript accessing Python values
        console.log(window.pyProperty);  // Should output: Python property value
        console.log(window.pyFunction("test"));  // Should output: Python function called with test
    </script>
</body>
</html>
```

#### JavaScript Calling Python Tests
```html
<!-- Tests/WebTests/Layout/Python/Interop/js-calls-python.html -->
<!DOCTYPE html>
<html>
<head>
    <title>JavaScript Calls Python Test</title>
</head>
<body>
    <div id="py-output"></div>
    
    <script type="python">
        # Python function to be called from JavaScript
        def pyFunction(message):
            print("Called from JavaScript:", message)
            return "Hello from Python!"
        
        # Python object to be accessed from JavaScript
        class PyClass:
            def __init__(self):
                self.property = "Python property value"
            
            def method(self):
                return "Python method called"
        
        window.pyFunction = pyFunction
        window.pyObject = PyClass()
    </script>
    
    <script>
        // JavaScript calling Python
        var result = window.pyFunction("Hello from JavaScript");
        console.log(result);  // Should output: Hello from Python!
        
        // JavaScript accessing Python object
        var pyProp = window.pyObject.property;
        var pyMethodResult = window.pyObject.method();
        console.log("Py property: " + pyProp + ", Py method: " + pyMethodResult);
        
        // JavaScript setting Python values
        window.jsProperty = "JavaScript property value";
        window.jsFunction = function(x) {
            return "JavaScript function called with " + x;
        };
    </script>
    
    <script type="python">
        # Python accessing JavaScript values
        print(window.jsProperty)  # Should output: JavaScript property value
        print(window.jsFunction("test"))  # Should output: JavaScript function called with test
    </script>
</body>
</html>
```

## Automated Testing Framework

### C++ Test Harness Integration

Integration with the existing test framework:

```cpp
// Tests/LibWeb/Scripting/Python/TestRunner.cpp
#include <LibTest/TestCase.h>
#include <LibWeb/Bindings/MainThreadVM.h>
#include <LibWeb/HTML/Scripting/PythonScript.h>
#include <LibWeb/HTML/Scripting/PythonEngine.h>

namespace Web::Tests {

class PythonTestHarness {
public:
    static void initialize_harness() {
        // Initialize the main thread VM with Python support
        Web::Bindings::initialize_main_thread_vm(Web::Bindings::AgentType::SimilarOriginWindow);
        
        // Verify Python engine is initialized
        VERIFY(HTML::PythonEngine::is_initialized());
    }
    
    static void shutdown_harness() {
        // Shut down Python engine
        HTML::PythonEngine::shutdown();
    }
    
    static Test::TestCaseResult run_python_test_case(Function<Test::TestCaseResult()> test_function) {
        // Ensure Python engine is available
        if (!HTML::PythonEngine::is_initialized()) {
            HTML::PythonEngine::initialize();
        }
        
        // Run the test
        return test_function();
    }
};

}
```

### Test Suite Organization

Organize tests into logical categories:

```
Tests/
├── LibWeb/
│   ├── Scripting/
│   │   ├── Python/
│   │   │   ├── BasicExecutionTest.cpp
│   │   │   ├── DOMAPITest.cpp
│   │   │   ├── SecurityTest.cpp
│   │   │   ├── CrossLanguageTest.cpp
│   │   │   ├── PerformanceTest.cpp
│   │   │   └── CMakeLists.txt
│   │   └── CMakeLists.txt
│   └── HTML/
│       ├── Scripting/
│       │   ├── HTMLScriptElementTest.cpp
│       │   ├── ScriptExecutionTest.cpp
│       │   └── CMakeLists.txt
│       └── CMakeLists.txt
└── WebTests/
    ├── Layout/
    │   ├── Python/
    │   │   ├── basic-rendering.html
    │   │   ├── css-selectors.html
    │   │   ├── event-handling.html
    │   │   └── Interop/
    │   │       ├── python-calls-js.html
    │   │       ├── js-calls-python.html
    │   │       └── mixed-language.html
    │   └── CMakeLists.txt
    └── CMakeLists.txt
```

## Manual Testing Procedures

### Browser-Based Testing

1. **Load Test HTML Files**
   ```bash
   ./Build/release/bin/Ladybird file:///path/to/test_python.html
   ```

2. **Console Verification**
   - Check that Python output appears in browser console
   - Verify DOM modifications are applied
   - Confirm error messages are properly displayed

3. **Developer Tools Inspection**
   - Use inspector to verify element creation
   - Check JavaScript console for cross-language messages
   - Monitor network requests from Python scripts

### Debugging Tools

1. **Python Debugger Integration**
   ```bash
   gdb --args ./Build/release/bin/Ladybird file:///path/to/test_python.html
   ```

2. **Logging and Tracing**
   - Enable HTML_SCRIPT_DEBUG for verbose output
   - Use dbgln() statements for tracing execution
   - Monitor GIL acquisition/release patterns

3. **Memory Analysis**
   - Use valgrind for memory leak detection
   - Monitor reference counting in Python objects
   - Check for circular references

## Continuous Integration Testing

### GitHub Actions Workflow

```yaml
# .github/workflows/python-integration.yml
name: Python Integration Tests

on:
  push:
    branches: [ master ]
  pull_request:
    branches: [ master ]

jobs:
  python-tests:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Install dependencies
      run: |
        sudo apt update
        sudo apt install python3-dev libpython3-dev
        
    - name: Build Ladybird
      run: |
        cmake --preset default -B Build/release
        cmake --build --preset default Build/release
        
    - name: Run Python unit tests
      run: |
        cd Build/release
        ctest -R PythonScriptTests
        
    - name: Run Python layout tests
      run: |
        cd Build/release
        ./bin/ladybird-layout-test --test-dir ../Tests/WebTests/Layout/Python
```

## Performance Testing

### Benchmark Suite

```python
# Tests/Benchmarks/PythonBenchmark.py
import time
import statistics

def benchmark_python_vs_js():
    """Compare Python and JavaScript execution performance"""
    
    # Test computation-intensive operations
    def python_computation():
        start = time.time()
        result = sum(i**2 for i in range(100000))
        end = time.time()
        return end - start, result
    
    # Test DOM manipulation
    def python_dom_manipulation():
        start = time.time()
        for i in range(1000):
            div = document.create_element("div")
            div.text = f"Element {i}"
            document.body.append_child(div)
        end = time.time()
        return end - start
    
    # Run benchmarks
    computation_times = []
    dom_times = []
    
    for _ in range(10):
        comp_time, _ = python_computation()
        computation_times.append(comp_time)
        
        dom_time = python_dom_manipulation()
        dom_times.append(dom_time)
    
    print(f"Computation average: {statistics.mean(computation_times):.4f}s")
    print(f"DOM manipulation average: {statistics.mean(dom_times):.4f}s")

# Run benchmark
benchmark_python_vs_js()
```

### Memory Usage Testing

```python
# Tests/Benchmarks/PythonMemoryTest.py
import gc

def test_memory_usage():
    """Test memory consumption patterns"""
    
    # Create many objects
    elements = []
    for i in range(10000):
        element = document.create_element("div")
        element.set_attribute("id", f"element-{i}")
        elements.append(element)
    
    # Measure memory before cleanup
    gc.collect()  # Force garbage collection
    print(f"Created {len(elements)} elements")
    
    # Clean up
    elements.clear()
    gc.collect()
    print("Cleaned up elements")

test_memory_usage()
```

## Security Testing

### Penetration Testing Scenarios

1. **Module Import Attempts**
   ```python
   # Attempt to import dangerous modules
   dangerous_modules = [
       "os", "sys", "subprocess", "socket", "urllib", "urllib2", "httplib", "http.client",
       "ftplib", "telnetlib", "smtplib", "pickle", "cPickle", "shelve", "marshal", "imp",
       "importlib", "compileall", "py_compile", "shutil", "glob", "requests", "urllib3",
       "httpx", "aiohttp", "xml", "xmlrpclib", "urllib.request", "urllib.parse", "urllib.error"
   ]
   
   for module in dangerous_modules:
       try:
           __import__(module)
           print(f"ERROR: Successfully imported {module}")
       except ImportError:
           print(f"SUCCESS: {module} import blocked")
   ```

2. **Filesystem Access Attempts**
   ```python
   # Attempt various filesystem operations
   import os
   
   # These should all fail
   try:
       os.listdir("/")
       print("ERROR: Directory listing succeeded")
   except:
       print("SUCCESS: Directory listing blocked")
       
   try:
       open("/etc/passwd", "r")
       print("ERROR: File access succeeded")
   except:
       print("SUCCESS: File access blocked")
   ```

3. **Network Access Tests**
   ```python
   # Test cross-origin restrictions
   import urllib.request
   
   try:
       response = urllib.request.urlopen("http://example.com")
       print("ERROR: Cross-origin request succeeded")
   except:
       print("SUCCESS: Cross-origin request blocked")
   ```

## Debugging and Troubleshooting

### Common Issues and Solutions

1. **Python Not Initializing**
   - Check that `Py_Initialize()` is called
   - Verify Python libraries are linked
   - Ensure Python development headers are available

2. **GIL Issues**
   - Always acquire GIL before Python operations
   - Release GIL after Python operations
   - Check for deadlocks in multithreaded scenarios

3. **Memory Leaks**
   - Monitor reference counting with `Py_INCREF`/`Py_DECREF`
   - Use Python's reference tracing features
   - Run tests with valgrind for leak detection

4. **Cross-Language Communication Failures**
   - Check value conversion routines
   - Verify object proxying is working
   - Test both directions of communication

### Logging and Debugging

Enable debug logging:
```cpp
// In code
dbgln_if(HTML_SCRIPT_DEBUG, "PythonScript: Executing script {}", filename());
```

Run with debug enabled:
```bash
export HTML_SCRIPT_DEBUG=1
./Build/release/bin/Ladybird file:///path/to/test_python.html
```

### Test Coverage Metrics

Track test coverage for Python integration:
- Unit test coverage: >90%
- Integration test coverage: >80%
- Layout test coverage: >70%
- Security test coverage: 100% for critical paths

Regular coverage reports:
```bash
# Generate coverage report
lcov --capture --directory Build/release --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

## Regression Testing

### Backward Compatibility Tests

Ensure that existing JavaScript functionality is not broken:
- All existing JavaScript tests still pass
- Mixed Python/JavaScript pages work correctly
- DOM API behavior is consistent

### Edge Case Tests

Test unusual scenarios:
- Empty Python scripts
- Python scripts with syntax errors
- Large Python scripts
- Python scripts with infinite loops
- Concurrent Python script execution
- Nested Python script tags
- Dynamically added Python scripts

### Stress Tests

Test under heavy load:
- Multiple Python scripts on a single page
- Rapid DOM manipulation from Python
- Complex cross-language communication
- Memory-intensive Python operations
- Long-running Python scripts

This comprehensive testing strategy ensures that the Python integration in Ladybird browser is reliable, secure, and performs well while maintaining backward compatibility with existing JavaScript functionality.