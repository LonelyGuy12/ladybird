# Python Integration Documentation

## Overview

This directory contains comprehensive documentation for the Python integration in Ladybird browser. The integration enables Python to be used as a first-class scripting language alongside JavaScript.

## Documentation Files

### [Overview](overview.md)
- Project goals and objectives
- High-level architecture description
- Implementation status

### [Technical Architecture](architecture.md)
- Detailed system architecture
- Component interactions
- Implementation flow

### [Implementation Details](implementation.md)
- Core implementation files
- Build system changes
- Key implementation patterns
- Testing examples

### [Build System Integration](build_system.md)
- CMake configuration changes
- Dependency requirements
- Build process instructions
- Troubleshooting guide

### [Security Model](security.md)
- Sandboxing implementation
- Built-in function restrictions
- Module import controls
- Filesystem and network access restrictions

### [DOM API Bindings](dom_bindings.md)
- Python-idiomatic DOM APIs
- Object wrapping implementation
- Method signatures and examples
- Cross-language access

### [Cross-Language Communication](cross_language_communication.md)
- Bridge implementation details
- Value conversion mechanisms
- Function call marshalling
- Exception handling

### [Testing Strategy](testing.md)
- Unit testing framework
- Integration testing approaches
- Layout tests
- Security penetration testing
- Performance benchmarks

### [Performance Optimization](performance.md)
- Current performance challenges
- Optimization strategies
- JIT compilation approaches
- Profiling and monitoring
- Benchmark comparisons

## Implementation Status

The Python integration is largely complete with these components implemented:

- ✅ Python execution environment in WebContent process
- ✅ HTML parser modifications to recognize Python script tags
- ✅ Python-idiomatic bindings for DOM APIs
- ✅ Security model with sandboxing
- ✅ Cross-language communication between Python and JavaScript
- ✅ Build system integration with CPython

## Key Features

1. **HTML Script Support**
   ```html
   <script type="python">
       document.find("#output").text = "Hello from Python!"
   </script>
   ```

2. **DOM API Access**
   - Pythonic access to Document, Element, Window objects
   - CSS selector support with `document.select()` and `document.find()`
   - Familiar property access patterns (`element.text`, `element.html`)

3. **Security**
   - Sandboxed execution environment
   - Restricted built-in functions
   - Module import controls
   - Same-Origin Policy enforcement

4. **Cross-Language Bridge**
   - Call JavaScript functions from Python
   - Access JavaScript objects from Python
   - Share data between languages
   - Unified error handling

## Future Work

Areas for continued development:
- Performance optimization to match JavaScript execution speed
- Expanded DOM API coverage
- Enhanced debugging tools
- Better profiling and monitoring
- Additional security enhancements

## Getting Started

To use Python in Ladybird:
1. Ensure Python 3 development libraries are installed
2. Build Ladybird with Python support enabled
3. Write HTML with `<script type="python">` tags
4. Access DOM APIs through Python-idiomatic interfaces

Example:
```html
<!DOCTYPE html>
<html>
<head>
    <title>Python Test</title>
</head>
<body>
    <div id="output"></div>
    
    <script type="python">
        # Access DOM elements
        output = document.find("#output")
        output.text = "Hello from Python!"
        
        # Print to console
        print("Python script executed successfully")
        
        # Access window object
        window_title = window.document.title
        print(f"Window title: {window_title}")
    </script>
</body>
</html>
```