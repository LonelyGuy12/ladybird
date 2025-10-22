# Python Integration Build Fixes

This document outlines the fixes required to resolve build issues with the Python integration in Ladybird.

## Current Issues

The Python integration is encountering several build errors:

1. **Python C API Struct Initialization**
   - Incompatible struct initialization syntax
   - Missing field initializers
   - Designated initializer ordering issues

2. **JavaScript API Compatibility**
   - Missing or changed API methods
   - Type conversion errors
   - Reference type issues

3. **DOM Interface Changes**
   - Type conversion from C strings to StringView/FlyString
   - Non-const to const reference conversions
   - Missing wrapper cache support

## Fix Implementation

### 1. Python C API Struct Initialization

Fix all Python type object initializations to use consistent designated initializer syntax:

```cpp
// Before (problematic)
PyTypeObject s_type = {
    .tp_name = "web.Document",
    // Inconsistent initialization
};

// After (fixed)
PyTypeObject s_type = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    .tp_name = "web.Document",
    .tp_basicsize = sizeof(PythonDocumentObject),
    .tp_itemsize = 0,
    // All required fields properly initialized
};
```

### 2. JavaScript API Compatibility

Update all JavaScript API calls to use the current API:

```cpp
// Before (problematic)
auto property_key = JS::PropertyKey::symbol_or_string(vm, attr_str);
js_obj->put(property_key, js_value);

// After (fixed)
auto property_key = JS::PropertyKey::from_string(vm, attr_str);
if (property_key.is_error())
    return nullptr;
js_obj->set(property_key.release_value(), js_value, true);
```

### 3. DOM Interface Changes

Fix all DOM interface calls with proper type conversions:

```cpp
// Before (problematic)
auto element = self->document->query_selector(selector);

// After (fixed)
auto selector_view = StringView(selector);
auto result = self->document->query_selector(selector_view);
if (result.is_error() || !result.value())
    Py_RETURN_NONE;
```

### 4. Memory Management

Add proper wrapper cache to the Document class:

```cpp
// In Document.h
Bindings::PythonDOMWrapperCache* wrapper_cache() const { return m_python_wrapper_cache.ptr(); }
void set_wrapper_cache(NonnullOwnPtr<Bindings::PythonDOMWrapperCache> cache) { m_python_wrapper_cache = move(cache); }

private:
    // Near the end of the class
    OwnPtr<Bindings::PythonDOMWrapperCache> m_python_wrapper_cache;
```

## Implementation Files Fixed

We have fixed the following files:

1. `/Users/lonely/important/ladybird/Libraries/LibWeb/Bindings/PythonJSObjectWrapper.cpp.fix`
   - Fixed type object initialization
   - Updated JavaScript API calls
   - Added proper memory management

2. `/Users/lonely/important/ladybird/Libraries/LibWeb/Bindings/PythonJSBridge.cpp.fix`
   - Updated type conversion logic
   - Fixed JavaScript API usage
   - Improved error handling

3. `/Users/lonely/important/ladybird/Libraries/LibWeb/Bindings/PythonDOMBindings.cpp.fix`
   - Fixed DOM API usage
   - Updated type conversions
   - Added proper wrapper cache management

4. `/Users/lonely/important/ladybird/Libraries/LibWeb/Bindings/PythonDOMBindings.h.fix`
   - Updated method signatures to match implementation
   - Removed static type objects

5. `/Users/lonely/important/ladybird/Libraries/LibWeb/HTML/Scripting/IndependentPythonEngine.h.fix`
   - Added missing methods
   - Updated interfaces for better integration

## Additional Files Created

These new files provide necessary components:

1. `/Users/lonely/important/ladybird/Libraries/LibWeb/HTML/Scripting/PythonPerformanceMetrics.h`
   - Performance monitoring infrastructure

2. `/Users/lonely/important/ladybird/Libraries/LibWeb/HTML/Scripting/PythonPerformanceMetrics.cpp`
   - Implementation of performance metrics

3. `/Users/lonely/important/ladybird/Libraries/LibWeb/HTML/Scripting/PythonError.cpp`
   - Python exception handling and conversion

4. `/Users/lonely/important/ladybird/Tests/Python/PythonIntegrationTest.h`
   - Test harness for Python integration

5. `/Users/lonely/important/ladybird/Tests/Python/PythonIntegrationTest.cpp`
   - Test implementation for Python integration

## CMake Build System Fixes

Applied patches to fix the CMake build:

1. `/Users/lonely/important/ladybird/Libraries/LibWeb/CMakeLists.txt.patch`
   - Improved Python detection
   - Added all necessary source files
   - Updated include paths and linker options
   - Added test targets

## Setup Script

Created a setup script to apply all fixes:

1. `/Users/lonely/important/ladybird/scripts/setup_python_integration.sh`
   - Checks for Python development headers
   - Applies all patches
   - Configures CMake with Python support

## How to Apply These Fixes

1. Run the setup script:
   ```bash
   chmod +x scripts/setup_python_integration.sh
   ./scripts/setup_python_integration.sh
   ```

2. This will:
   - Check for Python development headers
   - Apply all patches and fixes
   - Configure CMake with Python support
   - Set up the build environment

3. Build Ladybird with Python support:
   ```bash
   make -C Build
   ```

## Troubleshooting

If you encounter build issues after applying these fixes:

1. **Missing Python Headers**
   - Install Python development packages: `sudo apt-get install python3-dev` (Ubuntu/Debian)
   - Or equivalent for your platform

2. **CMake Configuration Issues**
   - Ensure you're using CMake 3.15 or later
   - Set `ENABLE_PYTHON=ON` when configuring CMake

3. **Compilation Errors**
   - Check that all `.fix` files were correctly applied
   - Verify that the Python C API headers are properly found

4. **Linking Errors**
   - Ensure that the Python library is properly linked
   - Check for missing symbols in the Python library

## Testing the Integration

After building successfully, test the integration:

1. Run the Python integration tests:
   ```bash
   ./Build/Tests/Python/PythonIntegrationTest
   ```

2. Load a Python test page in Ladybird:
   ```bash
   ./Build/Ladybird/ladybird file:///path/to/ladybird/python_demo.html
   ```
