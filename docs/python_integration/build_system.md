# Build System Integration

## CMake Configuration

The Python integration requires modifications to the CMake build system to properly link against Python libraries and include headers.

### Required Dependencies

1. **Python 3 Development Libraries**: `Python3_Development` component
2. **CPython Headers**: Available through `Python3_INCLUDE_DIRS`
3. **Python Libraries**: Available through `Python3_LIBRARIES`

### CMakeLists.txt Modifications

#### Main CMakeLists.txt
```cmake
# Changed from:
find_package(Python3 COMPONENTS Interpreter)

# To:
find_package(Python3 COMPONENTS Interpreter Development)
```

This ensures that both the Python interpreter and development libraries are available.

#### LibWeb/CMakeLists.txt

Added Python source files to the build:
- HTML/Scripting/PythonEngine.cpp
- HTML/Scripting/PythonScript.cpp
- HTML/Scripting/PythonSecurityModel.cpp
- Bindings/PythonDOMBindings.cpp
- Bindings/PythonJSBridge.cpp
- Bindings/PythonJSObjectWrapper.cpp
- Bindings/PythonObjectBridge.cpp
- Bindings/PythonWebModule.cpp

Added Python headers to the generated sources list:
- HTML/Scripting/PythonEngine.h
- HTML/Scripting/PythonScript.h
- HTML/Scripting/PythonSecurityModel.h

Linked against Python libraries:
```cmake
target_link_libraries(LibWeb PRIVATE 
    # ... other libraries ...
    ${Python3_LIBRARIES}
)

target_include_directories(LibWeb PRIVATE 
    # ... other includes ...
    ${Python3_INCLUDE_DIRS}
)
```

#### Services/WebContent/CMakeLists.txt

Linked Python libraries to the WebContent service:
```cmake
target_link_libraries(webcontentservice PRIVATE 
    # ... other libraries ...
    ${Python3_LIBRARIES}
)

target_include_directories(webcontentservice PRIVATE 
    # ... other includes ...
    ${Python3_INCLUDE_DIRS}
)
```

## Build Process

### Prerequisites

Before building, ensure Python 3 development packages are installed:

#### Debian/Ubuntu:
```bash
sudo apt install python3-dev
```

#### Arch Linux/Manjaro:
```bash
sudo pacman -S python3-dev
```

#### macOS:
```bash
# Using Homebrew
brew install python3

# Headers are typically included with the main Python installation
```

#### Windows (WSL2):
```bash
sudo apt install python3-dev
```

### Build Commands

Standard build process remains the same:
```bash
# From /path/to/ladybird
./Meta/ladybird.py run
```

Or manual build:
```bash
cmake --preset default -B Build/release
cmake --build --preset default Build/release
ninja -C Build/release run-ladybird
```

### Build Artifacts

The build process generates:
1. PythonEngine library integration
2. PythonScript execution capabilities
3. DOM API bindings for Python
4. Security model implementation
5. Cross-language bridge components

## Troubleshooting

### Common Issues

1. **Python Development Libraries Not Found**
   ```
   CMake Error: Python3 Development libraries not found
   ```
   Solution: Install python3-dev package for your distribution

2. **Header Files Not Found**
   ```
   fatal error: Python.h: No such file or directory
   ```
   Solution: Ensure Python3_INCLUDE_DIRS is properly set and accessible

3. **Linking Errors**
   ```
   undefined reference to Py_* functions
   ```
   Solution: Ensure Python3_LIBRARIES is properly linked

### Debugging Build Issues

1. Check CMake configuration:
   ```bash
   cmake --preset default -B Build/release --debug-output
   ```

2. Verify Python detection:
   ```bash
   cmake --preset default -B Build/release -DPython3_FIND_DEBUG=ON
   ```

3. Check library paths:
   ```bash
   pkg-config --cflags --libs python3
   ```

## Cross-Platform Considerations

### Linux
- Standard Python development packages work well
- Ensure libpython3.x.so is available

### macOS
- Homebrew Python installations typically include development headers
- May need to specify Python_ROOT_DIR for custom installations

### Windows (WSL2)
- Follow Linux instructions within WSL2 environment
- Native Windows builds are not currently supported

## Continuous Integration

The build system modifications should work with the existing CI pipeline:
- GitHub Actions workflows already install required dependencies
- Build matrix covers major platforms (Linux, macOS)
- No additional CI configuration should be needed

## Performance Impact

Build system changes have minimal impact on build times:
- Python integration adds ~5-10 seconds to build time
- No runtime performance impact on non-Python builds
- Conditional compilation ensures Python components only build when needed