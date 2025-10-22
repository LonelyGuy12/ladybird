# Python Integration in Ladybird Browser

## Project Overview

This document describes the implementation of Python as a first-class scripting language in the Ladybird browser, serving as a complete reference for developers who will continue working on this project.

Ladybird is a web browser that aims to provide a modern, secure, and efficient browsing experience. As part of this effort, we're adding Python scripting support alongside JavaScript, allowing developers to write browser scripts in Python instead of JavaScript.

## Goals

1. Enable Python as the default scripting language in the browser (while maintaining JS compatibility)
2. Provide full DOM API access with Python-idiomatic syntax
3. Recognize `<script type="python">` tags in HTML
4. Implement a robust security model to prevent malicious Python code execution
5. Enable cross-language communication between Python and JavaScript
6. Ensure Python performance is competitive with JavaScript

## Architecture

The implementation follows the existing JavaScript architecture pattern in Ladybird:

- PythonScript class (analogous to ClassicScript) for script execution
- PythonEngine for managing the Python interpreter
- PythonDOMAPI for Python-idiomatic DOM bindings
- HTMLScriptElement modifications to recognize Python script types
- PythonSecurityModel for sandboxed execution
- PythonJSBridge for cross-language communication

## Key Components

### 1. PythonScript
Handles parsing and execution of Python scripts, similar to how ClassicScript handles JavaScript.

### 2. PythonEngine
Manages the Python interpreter lifecycle, initialization, and shutdown.

### 3. PythonDOMAPI
Provides Python-friendly bindings to DOM APIs, Window object, and other Web APIs.

### 4. HTML Integration
Modified HTML parser to recognize Python script tags with appropriate language attributes.

### 5. Security Model
Sandboxes Python execution to prevent malicious code from accessing system resources.

### 6. Cross-Language Bridge
Enables communication between Python and JavaScript code within the browser.

## Implementation Status

This implementation is largely complete with the following components in place:

- [x] Python execution environment in WebContent process
- [x] HTML parser modifications to recognize Python script tags
- [x] Python-idiomatic bindings for DOM APIs
- [x] Basic security model for Python execution
- [x] Cross-language communication infrastructure
- [ ] Performance optimization
- [ ] Comprehensive testing

## Files and Directory Structure

The implementation spans several directories within the Ladybird codebase:

```
Libraries/LibWeb/
├── HTML/
│   ├── Scripting/
│   │   ├── PythonEngine.h/cpp
│   │   ├── PythonScript.h/cpp
│   │   ├── PythonSecurityModel.h/cpp
│   │   └── (existing JavaScript files)
│   └── HTMLScriptElement.cpp (modified)
├── Bindings/
│   ├── PythonDOMBindings.h/cpp
│   ├── PythonJSBridge.h/cpp
│   ├── PythonJSObjectWrapper.h/cpp
│   ├── PythonObjectBridge.h/cpp
│   ├── PythonWebModule.h/cpp
│   └── MainThreadVM.cpp (modified)
└── CMakeLists.txt (modified)

Services/WebContent/
├── CMakeLists.txt (modified)
└── main.cpp (modified)

Build System/
└── CMakeLists.txt (modified)
```