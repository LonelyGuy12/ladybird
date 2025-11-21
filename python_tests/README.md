# 🐍 Python Integration Examples for Ladybird

This directory contains comprehensive examples demonstrating Python integration in the Ladybird web browser.

## 🔧 Recent Fixes (November 21, 2025)

### What Was Fixed:
✅ **Script Type Standardization**: All `<script type="python">` tags have been updated to `<script type="text/python">` for better standards compliance.

### Files Updated:
- ✅ `python_basic_test.html` - All 8 test scripts
- ✅ `python_dom_test.html` - All 4 test scripts
- ✅ `python_advanced_test.html` - All 6 test scripts
- ✅ `index.html` - Quick verification script
- ✨ **NEW**: `simple_test.html` - Minimal debugging test

### Why This Matters:
Both `type="python"` and `type="text/python"` are supported by Ladybird, but `text/python` follows web standards (like `text/javascript`). If tests weren't working, it's now easier to debug with the new `simple_test.html` file.

---

## 📁 Example Files

### 0. `simple_test.html` ⭐ **NEW - Start Here!**
**Minimal Python DOM test for debugging**
- ✅ Just 2 simple tests (arithmetic and strings)
- ✅ Clear visual feedback (boxes turn green on success)
- ✅ Minimal code - easy to understand and debug
- ✅ **Use this first** to verify Python integration works
- 👁️ Perfect for troubleshooting issues

### 1. `python_basic_test.html`
**Core Python functionality testing**
- Basic Python operations (arithmetic, strings)
- Data structures (lists, dictionaries)
- Control flow (loops, conditionals, functions)
- String manipulation and formatting
- Function definitions and recursion

### 2. `python_dom_test.html`
**DOM and browser integration testing**
- Window and document object access
- DOM element querying and manipulation
- Python-browser environment inspection
- Integration status reporting
- Error handling for DOM operations

### 3. `python_advanced_test.html`
**Advanced Python features and performance**
- Large-scale computations and performance testing
- Complex algorithms (Fibonacci, factorial)
- Object-oriented programming (classes, methods)
- Exception handling and error management
- Data processing and sorting algorithms
- Method chaining and advanced patterns

## 🚀 How to Use These Examples

### Running the Tests
1. **Start Ladybird browser** with your Python integration build
2. **Open any example file** in the browser
3. **Open Developer Console** (F12 or right-click → Inspect)
4. **Look for Python output** in the console

### Expected Results
- **Green ✓ symbols**: Successful tests
- **Red ✗ symbols**: Failed tests (investigate)
- **Blue ℹ️ symbols**: Informational messages
- **Performance metrics**: Execution times and statistics

### Troubleshooting
If tests fail:
1. **Check console errors** for Python syntax issues
2. **Verify Python integration** is enabled in build
3. **Ensure DOM bindings** are properly compiled
4. **Review browser compatibility** with Python features

## 🧪 Test Coverage

### ✅ Confirmed Working Features
- Python script execution in `<script type="text/python">` tags (also supports `type="python"`)
- Basic Python syntax and built-in functions
- Data structures and control flow
- Function definitions and recursion
- Classes and object-oriented programming
- Exception handling (try/except blocks)
- Performance timing and measurements
- DOM element finding with `document.find()`
- Element text content manipulation with `.text` property
- Attribute setting with `.set_attribute()`

### 🔄 Features Under Development
- DOM manipulation APIs (element.setAttribute, etc.)
- Browser API bindings (fetch, localStorage, etc.)
- Async/await support with browser event loop
- Python module imports and package management
- Cross-script communication and state sharing

## 📊 Performance Expectations

### Typical Benchmark Results
- **Simple operations**: < 1ms execution time
- **List comprehensions**: ~10-50ms for 100k items
- **Recursive algorithms**: Varies by complexity
- **DOM operations**: Depends on binding implementation

### Performance Tips
1. **Use list comprehensions** for better performance
2. **Avoid deep recursion** for large datasets
3. **Cache DOM references** when possible
4. **Profile performance** using `time.perf_counter()`

## 🛠️ Development Notes

### Creating New Examples
When adding new examples:
1. **Follow the existing structure** (test sections, console output)
2. **Include error handling** for incomplete features
3. **Add progress indicators** (✓, ✗, ℹ️)
4. **Document expected behavior** in comments
5. **Test across different scenarios** (success/failure cases)

### Integration Status
These examples are designed to work with the current Python integration in Ladybird. As features are added or APIs change, examples should be updated accordingly.

### Contributing
- **Test new Python features** by adding examples
- **Report integration issues** if examples fail
- **Suggest improvements** for test coverage
- **Document new capabilities** as they're implemented

## 🎯 Quick Start

1. **Clone/build Ladybird** with Python integration
2. **Start with `simple_test.html`** ⭐ to verify Python works (easiest)
3. **Try `python_basic_test.html`** to verify core functionality  
4. **Explore `python_advanced_test.html`** for comprehensive testing
5. **Experiment with `python_dom_test.html`** for browser integration
6. **Check Developer Console** (F12) for all output and results

---

**Note**: These examples reflect the current state of Python integration in Ladybird. Features marked as "in development" may not work fully until the corresponding APIs are implemented.
