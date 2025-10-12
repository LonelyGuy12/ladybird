# Python API Reference for Ladybird Browser

## Overview

This document provides a complete reference for the Python API available in the Ladybird browser. The API closely mirrors JavaScript DOM APIs but with Python-idiomatic syntax and conventions.

## Global Objects

### `document`
The Document object represents the HTML document being viewed.

```python
# Access the current document
doc = document
```

#### Properties
- `url` - Document URL
- `title` - Document title
- `body` - Document body element
- `head` - Document head element

#### Methods
- `select(selector)` - Query multiple elements using CSS selector
- `find(selector)` - Query single element using CSS selector
- `get_element_by_id(id)` - Get element by ID
- `get_elements_by_class_name(class_name)` - Get elements by class name
- `get_elements_by_tag_name(tag_name)` - Get elements by tag name
- `create_element(tag_name)` - Create new DOM element
- `create_text_node(text)` - Create new text node

### `window`
The Window object represents the browser window.

```python
# Access the current window
win = window
```

#### Properties
- `document` - Current document
- `location` - Current location
- `navigator` - Browser navigator
- `inner_width` - Window inner width
- `inner_height` - Window inner height

#### Methods
- `alert(message)` - Display alert dialog
- `confirm(message)` - Display confirmation dialog
- `prompt(message, default=None)` - Display prompt dialog
- `set_timeout(callback, delay)` - Schedule one-time callback
- `set_interval(callback, delay)` - Schedule repeating callback
- `clear_timeout(timer_id)` - Cancel timeout
- `clear_interval(interval_id)` - Cancel interval
- `fetch(url, options=None)` - Perform HTTP fetch request

### `console`
The Console object provides access to browser console functionality.

```python
# Log to console
console.log("Debug message")
console.error("Error message")
console.warn("Warning message")
```

#### Methods
- `log(*messages)` - Log messages to console
- `error(*messages)` - Log error messages to console
- `warn(*messages)` - Log warning messages to console
- `info(*messages)` - Log info messages to console
- `debug(*messages)` - Log debug messages to console
- `clear()` - Clear console

## DOM Objects

### `Element`
Represents a DOM element.

```python
# Get an element
element = document.find("#my-element")
```

#### Properties
- `tag_name` - Element tag name
- `text` - Text content
- `html` - HTML content
- `parent_element` - Parent element
- `children` - Child elements
- `first_element_child` - First child element
- `last_element_child` - Last child element
- `next_element_sibling` - Next sibling element
- `previous_element_sibling` - Previous sibling element
- `class_name` - Class attribute
- `id` - ID attribute

#### Methods
- `get_attribute(name)` - Get attribute value
- `set_attribute(name, value)` - Set attribute value
- `remove_attribute(name)` - Remove attribute
- `has_attribute(name)` - Check if element has attribute
- `append_child(child)` - Append child element
- `remove_child(child)` - Remove child element
- `replace_child(new_child, old_child)` - Replace child element
- `insert_before(new_node, reference_node)` - Insert node before reference
- `query_selector(selector)` - Find first child matching selector
- `query_selector_all(selector)` - Find all children matching selector
- `add_event_listener(event_type, callback)` - Add event listener
- `remove_event_listener(event_type, callback)` - Remove event listener

### `Document`
Represents the HTML document.

Inherits from Element.

#### Properties
All Element properties plus:
- `forms` - All form elements
- `images` - All image elements
- `links` - All link elements
- `scripts` - All script elements

#### Methods
All Element methods plus:
- `create_attribute(name)` - Create attribute
- `create_comment(data)` - Create comment
- `create_document_fragment()` - Create document fragment
- `get_elements_by_name(name)` - Get elements by name
- `import_node(node, deep=False)` - Import node from another document
- `adopt_node(node)` - Adopt node from another document

### `Event`
Represents a DOM event.

```python
# Event handler
def click_handler(event):
    print("Element clicked!")
    event.prevent_default()
```

#### Properties
- `type` - Event type
- `target` - Event target
- `current_target` - Current target
- `bubbles` - Whether event bubbles
- `cancelable` - Whether event is cancelable
- `default_prevented` - Whether default is prevented
- `time_stamp` - Event timestamp

#### Methods
- `prevent_default()` - Prevent default event behavior
- `stop_propagation()` - Stop event propagation
- `stop_immediate_propagation()` - Stop immediate event propagation

## JavaScript Interoperability

### Accessing JavaScript Objects
```python
# Access JavaScript global object
js_window = window

# Access JavaScript properties
js_title = window.document.title

# Call JavaScript functions
result = window.some_js_function("argument")

# Set JavaScript values
window.pyProperty = "Python value"
```

### Calling JavaScript Functions from Python
```python
# Simple function call
result = window.alert("Hello from Python")

# Function with arguments
calc_result = window.calculate_sum(10, 20)

# Accessing JavaScript objects
js_obj = window.jsObject
js_value = js_obj.property
```

### Accessing Python Objects from JavaScript
```javascript
// Access Python global object
var py_window = window;

// Access Python properties
var py_title = window.document.title;

// Call Python functions
var result = window.python_function("argument");

// Set Python values
window.jsProperty = "JavaScript value";
```

## Security Restrictions

### Forbidden Operations
The following operations are blocked for security:

1. **Dangerous Module Imports**
   ```python
   # These will raise ImportError
   import os          # Blocked
   import sys         # Blocked
   import subprocess  # Blocked
   import socket      # Blocked
   ```

2. **Filesystem Access**
   ```python
   # These will raise exceptions
   f = open("/etc/passwd", "r")  # Blocked
   import os
   os.listdir("/")               # Blocked
   ```

3. **Network Restrictions**
   ```python
   # Cross-origin requests may be blocked
   import urllib.request
   response = urllib.request.urlopen("http://different-domain.com")  # May be blocked
   ```

### Allowed Operations
The following operations are permitted:

1. **Safe Module Imports**
   ```python
   # These are allowed
   import json        # Allowed
   import datetime    # Allowed
   import math        # Allowed
   import random      # Allowed
   import re          # Allowed
   ```

2. **DOM Manipulation**
   ```python
   # These are allowed
   element = document.find("#my-element")
   element.text = "New text"
   element.set_attribute("class", "new-class")
   ```

3. **Console Output**
   ```python
   # These are allowed
   print("Debug message")
   console.log("Console message")
   ```

## Best Practices

### 1. Efficient DOM Access
```python
# Good: Cache frequently accessed elements
button = document.find("#submit-button")
for i in range(100):
    button.text = f"Count: {i}"

# Avoid: Repeated DOM queries
for i in range(100):
    document.find("#submit-button").text = f"Count: {i}"  # Inefficient
```

### 2. Event Handler Management
```python
# Good: Define handlers as functions
def click_handler(event):
    print("Button clicked!")

button.add_event_listener("click", click_handler)

# Avoid: Lambda functions (harder to remove)
button.add_event_listener("click", lambda e: print("Clicked"))  # Hard to remove later
```

### 3. Error Handling
```python
# Good: Handle DOM errors gracefully
try:
    element = document.find("#might-not-exist")
    if element:
        element.text = "Found it!"
    else:
        print("Element not found")
except Exception as e:
    print(f"Error accessing element: {e}")
```

### 4. Memory Management
```python
# Good: Clean up references when done
large_data = None  # Explicitly clear large objects
```

## Python Language Features

### Supported Python Features
The following Python features are supported:
- Basic data types (int, float, str, bool, list, dict, tuple)
- Control flow (if, for, while, try/except)
- Functions and classes
- List/dict comprehensions
- String formatting
- Standard library modules (json, datetime, math, random, re, collections, itertools)

### Unsupported Python Features
The following Python features are not supported for security:
- File I/O operations
- System calls
- Network operations outside SOP
- Dangerous modules (os, sys, subprocess, etc.)
- Import of external packages
- Reflection/metaprogramming features

## Cross-Language Examples

### Python Calling JavaScript
```python
# Call JavaScript function
result = window.js_function("Hello from Python")

# Access JavaScript object
js_value = window.js_object.property

# Set JavaScript value
window.pyProperty = "Python value"

# Work with JavaScript arrays
js_array = window.js_array
for item in js_array:
    print(item)
```

### JavaScript Calling Python
```javascript
// Call Python function
var result = window.python_function("Hello from JavaScript");

// Access Python object
var pyValue = window.python_object.property;

// Set Python value
window.jsProperty = "JavaScript value";

// Work with Python lists
var pyList = window.python_list;
for (var i = 0; i < pyList.length; i++) {
    console.log(pyList[i]);
}
```

## Performance Tips

### 1. Minimize Cross-Language Calls
```python
# Bad: Many cross-language calls
for i in range(1000):
    window.js_function(i)

# Good: Batch operations
numbers = list(range(1000))
window.js_function_batch(numbers)
```

### 2. Use DOM APIs Efficiently
```python
# Bad: Modifying DOM in a loop
for i in range(1000):
    div = document.create_element("div")
    div.text = f"Item {i}"
    document.body.append_child(div)

# Good: Batch DOM modifications
fragment = document.create_document_fragment()
for i in range(1000):
    div = document.create_element("div")
    div.text = f"Item {i}"
    fragment.append_child(div)
document.body.append_child(fragment)
```

### 3. Cache Expensive Operations
```python
# Bad: Repeated expensive operations
def process_elements():
    elements = document.select(".expensive-selector")
    # Process elements
    return elements

# Good: Cache results
_element_cache = {}

def process_elements():
    selector = ".expensive-selector"
    if selector not in _element_cache:
        _element_cache[selector] = document.select(selector)
    elements = _element_cache[selector]
    # Process elements
    return elements
```

## Error Handling

### Python Exceptions
Python exceptions are converted to JavaScript errors:

```python
# Python code that raises exception
def faulty_function():
    raise ValueError("Something went wrong")

try:
    faulty_function()
except ValueError as e:
    print(f"Caught Python exception: {e}")
```

### JavaScript Errors in Python
JavaScript errors are converted to Python exceptions:

```python
# Python code that catches JavaScript error
try:
    window.faulty_js_function()
except Exception as e:
    print(f"Caught JavaScript error: {e}")
```

## Testing

### Unit Testing
```python
# Simple unit test
def test_document_find():
    # Create test element
    div = document.create_element("div")
    div.set_attribute("id", "test-id")
    document.body.append_child(div)
    
    # Test find method
    element = document.find("#test-id")
    assert element is not None
    assert element.tag_name == "DIV"
    assert element.get_attribute("id") == "test-id"

# Run test
test_document_find()
print("All tests passed!")
```

### Integration Testing
```python
# Integration test for cross-language communication
def test_cross_language_communication():
    # Define Python function accessible from JavaScript
    def python_function(arg):
        return f"Python received: {arg}"
    
    # Make it available globally
    window.python_function = python_function
    
    # JavaScript would call this as:
    # var result = window.python_function("test");
    # assert(result === "Python received: test");
    
    print("Cross-language communication test passed!")

test_cross_language_communication()
```

This API reference provides a comprehensive guide to using Python for web development in Ladybird browser, with familiar patterns that Python developers will find intuitive while maintaining the power and flexibility of web APIs.