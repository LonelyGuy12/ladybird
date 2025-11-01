# Ladybird Browser Test Suite

## Overview
This test suite provides comprehensive HTML pages to verify Ladybird browser functionality across different web technologies.

## Test Pages

### 1. **index.html** - Test Suite Landing Page
- Central hub for all tests
- Feature detection and status indicators
- Quick navigation to all test pages

### 2. **test_basic.html** - Basic HTML Rendering
Tests:
- Text formatting (bold, italic, code)
- Lists (ordered and unordered)
- Links and navigation
- Basic layout and styling

### 3. **test_css.html** - CSS Layout & Styling
Tests:
- CSS Grid layout
- Flexbox
- Gradients and backdrop filters
- Hover effects and transitions
- CSS animations
- Modern CSS features

### 4. **test_javascript.html** - JavaScript Execution
Tests:
- Basic JavaScript execution
- DOM manipulation
- Event handling
- Counter operations
- Async/Promise support
- Dynamic content creation

### 5. **test_forms.html** - HTML Forms
Tests:
- Text inputs
- Email validation
- Number inputs
- Select dropdowns
- Radio buttons
- Checkboxes
- Textareas
- Date inputs
- Form submission and validation

### 6. **test_python.html** - Python Integration
Tests:
- Python code execution (if available)
- Python-JavaScript bridge
- Fallback behavior when Python is not available
- JavaScript alternatives for Python features

### 7. **test_pyscript_benchmark.html** - PyScript Performance Benchmark
Tests:
- PyScript (Python via WebAssembly) execution
- Performance comparison: Python vs JavaScript
- Real-world benchmarks:
  - Fibonacci calculation
  - List processing (100k elements)
  - String concatenation (10k operations)
  - Array sorting (50k elements)
- Side-by-side performance metrics
- Comprehensive performance summary table

## How to Use

### Local Testing
1. Open the built Ladybird browser
2. Navigate to `file:///path/to/tests_python/index.html`
3. Click on any test card to begin testing

### Testing from Build Directory
```bash
# After building Ladybird
cd Build/release/bin
./Ladybird /path/to/ladybird/tests_python/index.html
```

### Expected Results

#### ✅ Success Indicators
- Green checkmarks (✅)
- "Success" messages
- Properly rendered content
- Functional interactive elements

#### ❌ Failure Indicators
- Red error messages
- Missing or broken functionality
- Console errors (check developer tools)

## Test Checklist

Use this checklist to verify browser functionality:

- [ ] **Basic HTML**
  - [ ] Text renders correctly
  - [ ] Lists display properly
  - [ ] Links are clickable
  - [ ] Styles apply correctly

- [ ] **CSS**
  - [ ] Grid layout works
  - [ ] Flexbox renders properly
  - [ ] Animations play smoothly
  - [ ] Hover effects trigger
  - [ ] Gradients display correctly

- [ ] **JavaScript**
  - [ ] Initial JS test shows success
  - [ ] Counter increments/decrements
  - [ ] List items can be added
  - [ ] Input echo works
  - [ ] Promise test completes

- [ ] **Forms**
  - [ ] All input types render
  - [ ] Form validation works
  - [ ] Form submission displays data
  - [ ] Checkboxes and radios work

- [ ] **Python** (if enabled)
  - [ ] Python code executes
  - [ ] Results display correctly
  - [ ] Fallback works if unavailable

- [ ] **PyScript Benchmark**
  - [ ] PyScript loads successfully
  - [ ] All 4 benchmarks run
  - [ ] Results show timing comparisons
  - [ ] Summary table updates
  - [ ] Performance metrics display correctly

## Known Limitations

1. **Python Integration**: Only available if Ladybird was built with `ENABLE_PYTHON=ON`
2. **Some CSS features**: May have limited support depending on build
3. **Advanced APIs**: WebGL, WebAudio, etc. are not tested here

## Troubleshooting

### No content displays
- Check console for JavaScript errors
- Verify file paths are correct
- Ensure Ladybird build is complete

### Python tests fail
- Python integration is optional
- Check if built with `ENABLE_PYTHON=ON`
- Fallback JavaScript behavior is normal

### Styling looks wrong
- Some CSS features may be in development
- Check browser console for CSS warnings
- Try basic HTML test first

## Adding New Tests

To add a new test page:

1. Create a new `.html` file in this directory
2. Follow the existing structure and styling
3. Add a link from `index.html`
4. Update this README with test description
5. Add cross-links from other test pages

## Browser Information

These tests work with:
- **Ladybird Browser** (primary target)
- Any modern web browser (for comparison)

## License

These test files are part of the Ladybird project.
See the main LICENSE file for details.
