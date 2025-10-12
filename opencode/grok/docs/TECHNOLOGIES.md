# Technology Stack

## Core Technologies

### Programming Languages

#### C++23 (Primary)
- **Standard**: ISO/IEC 14882:2023
- **Compilers**: GCC 14+, Clang 20+, MSVC 19.3+
- **Features Used**:
  - Modules (planned)
  - Coroutines (planned)
  - Concepts
  - Ranges
  - Three-way comparison
  - Designated initializers
  - Lambda improvements

#### Python 3.7+ (New Addition)
- **Integration**: CPython embedding
- **Version Support**: 3.7, 3.8, 3.9, 3.10, 3.11, 3.12
- **Features**: Subinterpreters, GIL management, C API
- **Extensions**: C extensions support, native modules

#### Swift (Utilities)
- **Usage**: Test utilities, build scripts
- **Version**: 5.9+
- **Integration**: Xcode toolchain compatibility

### Build System

#### CMake 3.25+
- **Presets**: Development, Release, Sanitizer, Coverage
- **Generators**: Ninja (primary), Make, Visual Studio
- **Features**:
  - FetchContent for dependencies
  - Cross-platform configuration
  - Custom targets and commands

#### vcpkg (Dependency Management)
- **Manifest Mode**: vcpkg.json configuration
- **Platforms**: Windows, macOS, Linux, Android
- **Key Dependencies**:
  - Qt6 (UI framework)
  - Skia (2D graphics)
  - FFmpeg (multimedia)
  - OpenSSL (cryptography)
  - ICU (internationalization)

#### GN (Experimental)
- **Status**: Alternative build system
- **Usage**: Google-style build configuration
- **Compatibility**: Chromium build system heritage

### Core Libraries

#### AK (Core Utilities)
- **Purpose**: Fundamental data structures and algorithms
- **Key Components**:
  - String classes (String, StringView, ByteString)
  - Container classes (Vector, HashMap, HashSet)
  - Smart pointers (RefPtr, NonnullRefPtr)
  - Error handling (ErrorOr, Result)
  - Function utilities (Function, Bind)

#### LibWeb (Rendering Engine)
- **Architecture**: Independent web engine implementation
- **Components**:
  - HTML parser and DOM
  - CSS parser and cascade
  - Layout engine (box model)
  - Painting (Skia integration)
  - JavaScript integration (LibJS)
  - Python integration (new)

#### LibJS (JavaScript Engine)
- **Type**: Bytecode interpreter
- **Features**:
  - ECMAScript 2023 compliance
  - Bytecode compilation
  - Garbage collection
  - Module support
  - Web API bindings

#### LibCore (Platform Abstraction)
- **Purpose**: Cross-platform OS interface
- **Modules**:
  - Event loop (platform-specific)
  - Threading primitives
  - File system operations
  - Network abstractions
  - Process management

### Graphics and Media

#### Skia (2D Graphics)
- **Version**: Latest stable
- **Integration**: Custom painting backend
- **Features**: Hardware acceleration, text rendering
- **Backends**: OpenGL, Vulkan, Metal, Direct3D

#### FFmpeg (Multimedia)
- **Components**: libavcodec, libavformat, libavutil
- **Usage**: Audio/video decoding
- **Codecs**: MP3, AAC, H.264, VP8/VP9, AV1

#### SDL3 (Gamepad API)
- **Purpose**: Game controller support
- **Integration**: Web Gamepad API implementation

### Security and Cryptography

#### OpenSSL (TLS/Crypto)
- **Version**: 3.x series
- **Usage**: HTTPS, certificate validation
- **Algorithms**: AES, RSA, ECDSA, SHA-256

#### LibTLS (TLS Implementation)
- **Purpose**: Custom TLS library
- **Features**: Modern cipher suites, certificate handling

### Internationalization

#### ICU (Unicode Support)
- **Components**: libicuuc, libicui18n, libicudata
- **Features**:
  - Unicode text processing
  - Locale-aware formatting
  - Character encoding conversion
  - Collation and sorting

#### LibUnicode (Unicode Utilities)
- **Purpose**: High-level Unicode operations
- **Features**: Normalization, case conversion, text segmentation

### Networking

#### LibHTTP (HTTP Client)
- **Version**: HTTP/1.1, HTTP/2 (planned)
- **Features**: Request/response handling, redirects, cookies

#### LibRequests (High-level HTTP)
- **Purpose**: Browser request management
- **Features**: Resource loading, caching, CORS

#### cURL (Fallback HTTP)
- **Usage**: Alternative HTTP implementation
- **Integration**: Build-time option

### Compression and Archives

#### LibCompress
- **Algorithms**: gzip, deflate, brotli, zstd
- **Usage**: HTTP compression, resource decompression

### Testing Framework

#### LibTest (Custom Test Runner)
- **Features**:
  - Text-based test output
  - Layout test verification
  - Ref test comparison
  - Screenshot validation

#### GoogleTest (Unit Testing)
- **Integration**: C++ unit test framework
- **Usage**: Internal library testing

### Development Tools

#### Clang Tools
- **Clang-Format**: Code formatting (enforced)
- **Clang-Tidy**: Static analysis
- **Clangd**: Language server

#### Python Tools
- **Black**: Code formatting
- **Flake8**: Linting
- **MyPy**: Type checking (planned)

### Platform-Specific Technologies

#### Qt6 (Cross-Platform UI)
- **Modules**: QtCore, QtGui, QtWidgets, QtNetwork
- **Platforms**: Windows, macOS, Linux, Android
- **Features**: Native look and feel, accessibility

#### AppKit (macOS UI)
- **Framework**: Native macOS interface
- **Integration**: Cocoa interoperability
- **Features**: System integration, notifications

#### Android NDK (Mobile UI)
- **Components**: JNI, native activities
- **Integration**: Android system services

### Database and Storage

#### SQLite (Local Storage)
- **Usage**: Web Storage API implementation
- **Features**: IndexedDB, LocalStorage, SessionStorage

### Web Standards

#### W3C Specifications
- **HTML5**: Complete implementation
- **CSS3**: Cascading and layout
- **DOM Level 3**: Document manipulation
- **ECMAScript**: JavaScript language
- **Web APIs**: Fetch, WebSocket, WebGL (planned)

#### WHATWG Living Standards
- **HTML**: Parser and DOM
- **Fetch**: Network requests
- **URL**: Address parsing
- **Encoding**: Character encoding

### Performance and Profiling

#### Sanitizers
- **AddressSanitizer**: Memory error detection
- **UndefinedBehaviorSanitizer**: Undefined behavior detection
- **ThreadSanitizer**: Race condition detection

#### Profiling Tools
- **Perf**: Linux performance profiling
- **Instruments**: macOS profiling
- **VTune**: Intel performance analyzer

### Continuous Integration

#### GitHub Actions
- **Platforms**: Ubuntu, macOS, Windows
- **Tools**: CMake, Ninja, vcpkg
- **Testing**: Unit tests, integration tests

#### Build Configurations
- **Debug**: Full debugging symbols
- **Release**: Optimized builds
- **Sanitizer**: Memory safety checks

### Documentation

#### Doxygen (API Documentation)
- **Output**: HTML documentation
- **Integration**: CMake-generated

#### Sphinx (User Documentation)
- **Format**: reStructuredText
- **Output**: HTML, PDF

### Version Control

#### Git (Primary VCS)
- **Hosting**: GitHub
- **Workflow**: GitHub Flow
- **Hooks**: Pre-commit linting

#### Git LFS (Large Files)
- **Usage**: Test resources, screenshots
- **Integration**: Automatic handling

## Python Integration Technologies

### CPython Embedding
- **API**: Python C API
- **Features**:
  - Subinterpreter support
  - GIL management
  - Exception handling
  - Type conversion

### Python Standard Library
- **Modules**: Restricted access for security
- **Allowed**: Built-ins, math, random, json
- **Blocked**: os, sys, subprocess, importlib

### C Extensions
- **Support**: Native Python C extensions
- **Examples**: NumPy, SciPy, Pillow
- **Integration**: Dynamic loading support

### Package Management
- **pip**: Python package installer
- **wheels**: Pre-compiled packages
- **Virtualenv**: Isolated environments

## Future Technology Additions

### WebAssembly (WASM)
- **Integration**: WASI support
- **Usage**: High-performance computations
- **Languages**: C++, Rust compilation targets

### Rust Integration
- **Components**: Memory-safe libraries
- **Usage**: Security-critical components
- **FFI**: C++ interoperability

### Machine Learning
- **Frameworks**: TensorFlow, PyTorch (Python)
- **Usage**: AI-powered web features
- **Acceleration**: GPU computing support

### Blockchain/Web3
- **APIs**: Ethereum, IPFS integration
- **Security**: Cryptographic primitives
- **Standards**: Web3 wallet APIs

This technology stack provides a robust foundation for both the existing Ladybird browser and the new Python scripting capabilities, ensuring high performance, security, and cross-platform compatibility.