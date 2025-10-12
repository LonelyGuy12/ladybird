# Ladybird Browser Fork: Python-Enhanced Web Engine

## Project Overview

This repository is a fork of the [Ladybird Browser](https://github.com/LadybirdBrowser/ladybird), an independent web browser project implementing a novel web engine from scratch. This fork introduces groundbreaking innovation by integrating Python scripting capabilities directly into the web browser, enabling Python code to run natively in web pages alongside JavaScript.

### Core Innovation
- **Python as Primary Scripting**: Native Python execution replacing JavaScript as the primary web scripting language
- **JS-Independent Operation**: Python websites load and run without JavaScript dependency
- **Optional JS Bridge**: Bidirectional communication between Python and JavaScript for developers who need it
- **Performance Superiority**: Python execution optimized to be faster than JavaScript
- **DOM Manipulation**: Python access to web page elements and events
- **Security-First**: Leverages Ladybird's multi-process sandboxing for Python execution

### Key Features
- Independent web engine (LibWeb + LibJS) with Python extensions
- Multi-process architecture with pledge/unveil sandboxing
- Cross-platform support (Linux, macOS, Windows, Android)
- Modern web standards compliance
- Python ecosystem integration for web development
- Performance-optimized Python runtime (PyPy, WASM, assembly optimizations)
- Direct WASM loading APIs for JS-independent execution
- Modular architecture for size optimization and feature expansion

### Project Status
- **Base**: Ladybird v1.x (pre-alpha)
- **Fork Status**: Active development with Python integration
- **Build Status**: Partially integrated, requires debugging
- **Test Coverage**: Basic prototype with test_python.html

## Quick Start

### Prerequisites
- C++23 compiler (GCC 14+, Clang 20+, MSVC)
- CMake 3.25+
- Qt6 development libraries
- Python 3.7+ development headers
- vcpkg for dependency management

### Build Instructions
```bash
# Configure with vcpkg
cmake --preset default

# Build
cmake --build --preset default

# Run
./Meta/ladybird.py run
```

### Testing Python Features
```bash
# Open test page
./ladybird test_python.html
```

## Documentation Structure

- **[ARCHITECTURE.md](ARCHITECTURE.md)**: Detailed system architecture and components
- **[INNOVATIONS.md](INNOVATIONS.md)**: Python integration implementation details
- **[TECHNOLOGIES.md](TECHNOLOGIES.md)**: Technology stack and dependencies
- **[FUTURE_PLANS.md](FUTURE_PLANS.md)**: Development roadmap and goals
- **[IMPLEMENTATION_STATUS.md](IMPLEMENTATION_STATUS.md)**: Current progress and known issues
- **[PERFORMANCE.md](PERFORMANCE.md)**: Optimization strategies and benchmarks
- **[REQUIREMENTS.md](REQUIREMENTS.md)**: System prerequisites and compatibility

## Contributing

This fork welcomes contributions to the Python integration. See the original Ladybird repository for general contribution guidelines.

### Areas of Interest
- Python-JavaScript bridge optimization
- Security hardening for Python execution
- Performance benchmarking
- Cross-platform Python support
- Web API extensions for Python

## License

BSD-2-Clause (same as original Ladybird)

## Acknowledgments

- Original Ladybird team for the independent web engine
- Python community for the powerful scripting language
- Web standards organizations for guiding implementation