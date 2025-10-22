# System Architecture

## Overview

Ladybird implements a multi-process browser architecture with aggressive sandboxing, extended in this fork to support Python scripting execution. The system is designed for security, performance, and standards compliance.

## Process Architecture

### Core Processes

1. **Browser (UI Process)**
   - Location: `UI/Qt/main.cpp`, `UI/AppKit/`, `UI/Android/`
   - Responsibilities: User interface, window management, tab coordination
   - Technologies: Qt6 (primary), AppKit (macOS), Android NDK
   - Security: Runs as unprivileged user

2. **WebContent (Renderer Process)**
   - Location: `Services/WebContent/main.cpp`
   - Responsibilities: HTML parsing, CSS layout, JavaScript/Python execution
   - Technologies: LibWeb (rendering), LibJS (JavaScript), Python Engine (new)
   - Security: Fresh process per tab, pledge/unveil sandboxing
   - Isolation: One process per tab for site isolation

3. **RequestServer (Network Process)**
   - Location: `Services/RequestServer/`
   - Responsibilities: HTTP requests, resource fetching
   - Technologies: LibHTTP, LibTLS, LibRequests
   - Security: Shared process with sandboxing

4. **ImageDecoder (Media Process)**
   - Location: `Services/ImageDecoder/`
   - Responsibilities: Image decoding and processing
   - Technologies: LibGfx, FFmpeg integration
   - Security: Fresh process per image for isolation

5. **WebDriver (Automation Process)**
   - Location: `Services/WebDriver/`
   - Responsibilities: Browser automation and testing
   - Technologies: WebDriver protocol implementation

6. **WebWorker (Worker Process)**
   - Location: `Services/WebWorker/`
   - Responsibilities: Web Worker execution environment
   - Technologies: LibJS with worker-specific realm

### Python Integration Architecture

#### Python Engine Layer
- **PythonEngine** (`Libraries/LibWeb/HTML/Scripting/PythonEngine.h`)
  - Manages Python interpreter lifecycle
  - Creates isolated subinterpreters per execution context
  - Handles GIL (Global Interpreter Lock) management

#### Binding Layer
- **PythonDOMBindings** (`Libraries/LibWeb/Bindings/PythonDOMBindings.h`)
  - Python wrappers for DOM objects (Document, Element, Window)
  - Type conversion between Python and C++ DOM representations
  - Memory management with reference counting

- **PythonJSBridge** (`Libraries/LibWeb/Bindings/PythonJSBridge.h`)
  - Bidirectional communication between Python and JavaScript
  - Type conversion (PyObject ↔ JS::Value)
  - Function call proxying across language boundaries

#### Script Execution Pipeline

```
HTML Parser → Script Detection → Language Identification
    ↓
Python Script? → PythonEngine → Subinterpreter Creation
    ↓
DOM Binding Setup → JS Bridge Initialization
    ↓
Code Execution → Result Processing → DOM Updates
```

## Component Architecture

### LibWeb (Rendering Engine)
- **DOM Module**: Document Object Model implementation
- **CSS Module**: Style computation and cascading
- **Layout Module**: Box model and positioning
- **Painting Module**: Skia-based rendering
- **HTML Module**: Parser and element implementations
- **Scripting Module**: JavaScript and Python execution (extended)

### LibJS (JavaScript Engine)
- **Bytecode Interpreter**: ECMAScript execution
- **AST Parser**: JavaScript syntax analysis
- **Runtime**: Object model and standard library
- **Realm**: Execution context isolation

### LibCore (Platform Abstraction)
- **Event Loop**: Cross-platform event handling
- **Threading**: Thread management and synchronization
- **IPC**: Inter-process communication
- **File System**: Platform-agnostic file operations

### UI Layer
- **Qt Frontend**: Cross-platform desktop interface
- **AppKit Frontend**: Native macOS integration
- **Android Frontend**: Mobile platform support

## Data Flow

### Page Loading Sequence

1. **Navigation Request** (UI → RequestServer)
2. **Resource Fetching** (RequestServer → Network)
3. **HTML Parsing** (WebContent → LibWeb)
4. **DOM Construction** (LibWeb)
5. **Style Computation** (CSS)
6. **Layout Calculation** (Layout Engine)
7. **Script Execution** (JavaScript/Python)
8. **Painting** (Skia → GPU)
9. **Display** (UI Process)

### Python Script Execution

1. **Script Tag Detection** (`<script type="python">`)
2. **PythonEngine Initialization**
3. **Subinterpreter Creation** (for isolation)
4. **DOM Binding Injection** (document, window objects)
5. **JS Bridge Setup** (cross-language access)
6. **Code Compilation and Execution**
7. **Result Processing** (DOM updates, console output)
8. **Cleanup** (subinterpreter destruction)

## Security Model

### Process Isolation
- Each tab runs in separate WebContent process
- Network operations in dedicated RequestServer
- Image decoding in fresh processes
- All processes run as unprivileged users

### Sandboxing Mechanisms
- **pledge/unveil**: OpenBSD-inspired syscall restrictions
- **Seccomp**: Linux syscall filtering (planned)
- **Site Isolation**: Per-origin process separation

### Python Security
- **Subinterpreter Isolation**: Separate Python contexts per script
- **GIL Management**: Prevents race conditions
- **Resource Limits**: Memory and CPU restrictions (planned)
- **Module Restrictions**: Limited standard library access

## Memory Management

### Garbage Collection
- **LibJS Heap**: Mark-and-sweep for JavaScript objects
- **LibWeb Heap**: Reference counting with cycle detection
- **Python Heap**: CPython's reference counting + cyclic GC

### Smart Pointers
- **GC::Ref<T>**: Garbage-collected references
- **GC::Ptr<T>**: Nullable garbage-collected pointers
- **AK::RefPtr<T>**: Reference-counted pointers
- **AK::NonnullRefPtr<T>**: Non-null reference-counted pointers

## Inter-Process Communication

### IPC Protocol
- **Message Passing**: Synchronous and asynchronous messages
- **Serialization**: Custom binary protocol
- **Endpoints**: Strongly typed message handlers

### Key IPC Channels
- UI ↔ WebContent: Rendering commands, input events
- WebContent ↔ RequestServer: Resource requests
- WebContent ↔ ImageDecoder: Image processing
- UI ↔ All: Process management and coordination

## Build System

### CMake Configuration
- **Presets**: Development, release, sanitizer builds
- **vcpkg Integration**: Dependency management
- **Cross-Platform**: Windows, macOS, Linux, Android

### Dependency Graph
```
Ladybird
├── LibWeb (Rendering)
│   ├── LibJS (JavaScript)
│   ├── LibGfx (Graphics)
│   └── LibUnicode (Text)
├── LibCore (Platform)
├── Services (Processes)
└── UI (Frontends)
```

## Testing Architecture

### Test Types
- **Text Tests**: JavaScript API verification
- **Layout Tests**: DOM structure validation
- **Ref Tests**: Visual comparison
- **Screenshot Tests**: Pixel-perfect rendering
- **Python Integration Tests**: Cross-language functionality

### Test Infrastructure
- **LibTest**: Custom test runner
- **WPT Integration**: Web Platform Tests
- **CI Pipeline**: GitHub Actions with sanitizers