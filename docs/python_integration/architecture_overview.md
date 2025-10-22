# Python Integration Architecture Overview

## Independence from JavaScript

The Ladybird Python integration is designed as a fully independent subsystem that can operate without any JavaScript dependencies. This document describes the high-level architecture and components.

## Core Components

### 1. Independent Python Engine

The `IndependentPythonEngine` class provides a complete Python execution environment that:

- Initializes and manages a Python interpreter
- Executes Python code with high-performance optimizations
- Handles Python-to-browser communication directly
- Monitors performance with detailed metrics
- Implements caching and optimization techniques

Unlike traditional browser scripting, this engine does not depend on JavaScript for its operation. It interfaces directly with the browser's core APIs.

### 2. Python Security Model

The security model (`PythonSecurityModel`) ensures that Python code runs safely within the browser by:

- Implementing strong sandboxing and isolation
- Restricting access to dangerous modules and functions
- Enforcing resource limits (CPU, memory, stack depth)
- Integrating with the browser's Content Security Policy
- Validating Python code for dangerous patterns
- Applying Same-Origin Policy to network requests

### 3. Python Standard Library

The `PythonStdLib` provides optimized implementations of Python's standard library modules:

- Core modules like `json`, `datetime`, and `math`
- Asynchronous programming via `asyncio`
- Optimized data processing using browser's native capabilities
- Special browser-specific modules (`dom`, `browser`, `webapi`)

### 4. Browser API Bindings

Direct Python bindings to browser APIs (`PythonWebModule`) allow:

- DOM manipulation and querying
- Fetch API for network requests
- Local storage operations
- Event handling
- Performance and profiling interfaces
- Timers and callbacks

### 5. Optional JavaScript Bridge

An optional bridge (`OptionalPythonJSBridge`) enables communication between Python and JavaScript when needed:

- Message-based communication instead of direct references
- Controlled serialization/deserialization for type safety
- Event system for cross-language notifications
- Performance metrics for bridge operations

## Architecture Diagram

```
┌───────────────────────────────────────────────────────────┐
│                       Web Content                         │
│                                                           │
│  ┌─────────────┐     ┌──────────────┐    ┌─────────────┐  │
│  │   Python    │     │   Optional   │    │  JavaScript │  │
│  │   Script    │     │ Python-JS    │    │   Script    │  │
│  │   <script   │     │   Bridge     │    │   <script>  │  │
│  │  type=python>│     │ (on demand) │    │             │  │
│  └──────┬──────┘     └──────┬───────┘    └──────┬──────┘  │
│         │                   │                    │         │
└─────────┼───────────────────┼────────────────────┼─────────┘
          │                   │                    │
┌─────────┼───────────────────┼────────────────────┼─────────┐
│         │                   │                    │         │
│  ┌──────▼──────┐     ┌─────▼────────┐    ┌──────▼──────┐  │
│  │ Independent │     │    Bridge    │    │    JS VM    │  │
│  │   Python    │     │ Communication│    │             │  │
│  │   Engine    │◄────┤   Manager    ├────►             │  │
│  │             │     │              │    │             │  │
│  └──────┬──────┘     └──────────────┘    └──────┬──────┘  │
│         │                                        │         │
│  ┌──────▼──────┐                         ┌──────▼──────┐  │
│  │   Python    │                         │     JS      │  │
│  │  Security   │                         │   Security  │  │
│  │   Model     │                         │    Model    │  │
│  └──────┬──────┘                         └──────┬──────┘  │
│         │                                        │         │
│  ┌──────▼──────┐                         ┌──────▼──────┐  │
│  │   Python    │                         │     JS      │  │
│  │  Standard   │                         │   Standard  │  │
│  │  Library    │                         │  Library    │  │
│  └──────┬──────┘                         └──────┬──────┘  │
│         │                                        │         │
└─────────┼────────────────────────────────────────┼─────────┘
          │                                        │
┌─────────┼────────────────────────────────────────┼─────────┐
│         │                                        │         │
│  ┌──────▼────────────────────────────────────────▼──────┐  │
│  │                   Browser Engine                     │  │
│  │                                                      │  │
│  │  ┌────────────┐  ┌────────────┐  ┌────────────────┐  │  │
│  │  │    DOM     │  │   Fetch    │  │   WebStorage   │  │  │
│  │  │            │  │            │  │                │  │  │
│  │  └────────────┘  └────────────┘  └────────────────┘  │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                            │
└────────────────────────────────────────────────────────────┘
```

## Performance Advantages

Python integration offers several performance benefits:

1. **Direct Browser API Access**: No JS intermediaries for DOM operations
2. **Optimized Code Execution**: Compiled code caching and optimization
3. **Efficient Memory Management**: Controlled allocation and garbage collection
4. **Parallel Processing**: Better utilization of multi-core systems
5. **Reduced Overhead**: No constant marshalling between languages

## Security Measures

Security is a primary concern and includes:

1. **Isolated Execution**: Complete sandbox for Python scripts
2. **Resource Limits**: Prevents CPU/memory abuse
3. **Code Analysis**: Detects and blocks potentially harmful code
4. **Origin-Based Permissions**: Different capabilities based on source
5. **Audited Standard Library**: Only safe modules and functions

## Extension Points

The architecture allows for future enhancements:

1. **New Python Modules**: Additional standard library support
2. **Hardware Acceleration**: Integration with GPU and specialized processors
3. **Debugging Tools**: Script debugging and performance profiling
4. **Machine Learning**: Integration with ONNX Runtime or TensorFlow Lite
5. **Web Assembly**: Support for Python-compiled WASM modules
