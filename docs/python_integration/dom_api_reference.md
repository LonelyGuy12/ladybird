# Python DOM API Reference

## Overview

This document provides a complete reference for the Python DOM API bindings available in Ladybird browser. These bindings expose web APIs to Python scripts with Python-idiomatic syntax and conventions.

## Document Object

The `document` object provides access to the DOM tree and document-level operations.

### Properties

| Property | Description | Type |
|----------|-------------|------|
| `url` | Document URL | `str` |
| `title` | Document title | `str` |
| `body` | Document body element | `Element` |
| `head` | Document head element | `Element` |

### Methods

#### `select(selector)`
Query multiple elements using CSS selector syntax.

```python
# Select all div elements with class "content"
elements = document.select("div.content")

# Select all paragraph elements
paragraphs = document.select("p")

# Select elements by attribute
inputs = document.select("input[type='text']")
```

#### `find(selector)`
Query single element using CSS selector syntax. Returns first matching element or `None`.

```python
# Find element by ID
element = document.find("#my-id")

# Find first element with specific class
first_button = document.find(".button")

# Find element by tag name
first_header = document.find("h1")
```

#### `get_element_by_id(id)`
Get element by its ID attribute.

```python
# Get element with ID "content"
content = document.get_element_by_id("content")
```

#### `get_elements_by_class_name(class_name)`
Get elements by their class name.

```python
# Get all elements with class "highlight"
highlighted = document.get_elements_by_class_name("highlight")
```

#### `get_elements_by_tag_name(tag_name)`
Get elements by their tag name.

```python
# Get all div elements
divs = document.get_elements_by_tag_name("div")
```

#### `create_element(tag_name)`
Create a new DOM element.

```python
# Create new div element
new_div = document.create_element("div")

# Create new paragraph element
new_p = document.create_element("p")
```

#### `create_text_node(text)`
Create a new text node.

```python
# Create text node
text_node = document.create_text_node("Hello, world!")
```

## Element Object

The `Element` object represents DOM elements and provides methods for manipulation.

### Properties

| Property | Description | Type |
|----------|-------------|------|
| `tag_name` | Element tag name | `str` |
| `text` | Text content | `str` |
| `html` | HTML content | `str` |
| `parent_element` | Parent element | `Element` |
| `children` | Child elements | `list` |
| `first_element_child` | First child element | `Element` |
| `last_element_child` | Last child element | `Element` |
| `next_element_sibling` | Next sibling element | `Element` |
| `previous_element_sibling` | Previous sibling element | `Element` |
| `class_name` | Class attribute | `str` |
| `id` | ID attribute | `str` |

### Methods

#### `get_attribute(name)`
Get element attribute value.

```python
# Get class attribute
class_value = element.get_attribute("class")

# Get href attribute
href = link.get_attribute("href")
```

#### `set_attribute(name, value)`
Set element attribute value.

```python
# Set class attribute
element.set_attribute("class", "new-class")

# Set data attribute
element.set_attribute("data-value", "123")
```

#### `remove_attribute(name)`
Remove element attribute.

```python
# Remove class attribute
element.remove_attribute("class")
```

#### `has_attribute(name)`
Check if element has attribute.

```python
# Check if element has class attribute
if element.has_attribute("class"):
    print("Element has class attribute")
```

#### `append_child(child)`
Append child element.

```python
# Create and append new element
new_div = document.create_element("div")
element.append_child(new_div)
```

#### `remove_child(child)`
Remove child element.

```python
# Remove child element
element.remove_child(child_element)
```

#### `replace_child(new_child, old_child)`
Replace child element.

```python
# Replace existing child with new element
new_element = document.create_element("span")
element.replace_child(new_element, old_element)
```

#### `insert_before(new_node, reference_node)`
Insert node before reference node.

```python
# Insert new element before existing element
new_element = document.create_element("div")
element.insert_before(new_element, reference_element)
```

#### `query_selector(selector)`
Find first child element matching CSS selector.

```python
# Find first button inside element
button = element.query_selector("button")
```

#### `query_selector_all(selector)`
Find all child elements matching CSS selector.

```python
# Find all links inside element
links = element.query_selector_all("a")
```

#### `add_event_listener(event_type, callback)`
Add event listener to element.

```python
# Add click event listener
def click_handler(event):
    print("Element clicked!")

element.add_event_listener("click", click_handler)
```

#### `remove_event_listener(event_type, callback)`
Remove event listener from element.

```python
# Remove click event listener
element.remove_event_listener("click", click_handler)
```

## Window Object

The `Window` object provides access to browser functionality and global operations.

### Properties

| Property | Description | Type |
|----------|-------------|------|
| `document` | Current document | `Document` |
| `location` | Current location | `Location` |
| `navigator` | Browser navigator | `Navigator` |
| `inner_width` | Window inner width | `int` |
| `inner_height` | Window inner height | `int` |
| `outer_width` | Window outer width | `int` |
| `outer_height` | Window outer height | `int` |

### Methods

#### `alert(message)`
Display alert dialog.

```python
# Show alert message
window.alert("Hello from Python!")
```

#### `confirm(message)`
Display confirmation dialog.

```python
# Show confirmation dialog
result = window.confirm("Are you sure?")
if result:
    print("User confirmed")
else:
    print("User cancelled")
```

#### `prompt(message, default=None)`
Display prompt dialog.

```python
# Show prompt dialog
user_input = window.prompt("Enter your name:", "Anonymous")
print(f"User entered: {user_input}")
```

#### `set_timeout(callback, delay)`
Schedule one-time callback execution.

```python
# Schedule function to run after 1 second
def delayed_function():
    print("This runs after 1 second")

timer_id = window.set_timeout(delayed_function, 1000)
```

#### `set_interval(callback, delay)`
Schedule repeating callback execution.

```python
# Schedule function to run every 500ms
def repeating_function():
    print("This runs every 500ms")

interval_id = window.set_interval(repeating_function, 500)
```

#### `clear_timeout(timer_id)`
Cancel scheduled timeout.

```python
# Cancel timeout
window.clear_timeout(timer_id)
```

#### `clear_interval(interval_id)`
Cancel scheduled interval.

```python
# Cancel interval
window.clear_interval(interval_id)
```

#### `fetch(url, options=None)`
Perform HTTP fetch request.

```python
# Simple GET request
response = window.fetch("https://api.example.com/data")
data = response.json()

# POST request with options
options = {
    "method": "POST",
    "headers": {"Content-Type": "application/json"},
    "body": '{"key": "value"}'
}
response = window.fetch("https://api.example.com/data", options)
```

#### `console.log(message)`
Log message to browser console.

```python
# Log to console
window.console.log("Debug message")
```

#### `console.error(message)`
Log error to browser console.

```python
# Log error to console
window.console.error("Error message")
```

#### `console.warn(message)`
Log warning to browser console.

```python
# Log warning to console
window.console.warn("Warning message")
```

## Event Object

The `Event` object represents DOM events.

### Properties

| Property | Description | Type |
|----------|-------------|------|
| `type` | Event type | `str` |
| `target` | Event target | `Element` |
| `current_target` | Current target | `Element` |
| `bubbles` | Whether event bubbles | `bool` |
| `cancelable` | Whether event is cancelable | `bool` |
| `default_prevented` | Whether default is prevented | `bool` |
| `time_stamp` | Event timestamp | `float` |

### Methods

#### `prevent_default()`
Prevent default event behavior.

```python
# Prevent default form submission
def submit_handler(event):
    event.prevent_default()
    print("Form submission prevented")
```

#### `stop_propagation()`
Stop event propagation.

```python
# Stop event from bubbling up
def click_handler(event):
    event.stop_propagation()
    print("Event propagation stopped")
```

#### `stop_immediate_propagation()`
Stop immediate event propagation.

```python
# Stop all other listeners on this element
def click_handler(event):
    event.stop_immediate_propagation()
    print("Immediate propagation stopped")
```

## Console Object

The `console` object provides access to browser console functionality.

### Properties

None

### Methods

#### `log(*messages)`
Log messages to console.

```python
# Log multiple values
console.log("Value:", 42, "String:", "hello")
```

#### `error(*messages)`
Log error messages to console.

```python
# Log error
console.error("An error occurred:", exception)
```

#### `warn(*messages)`
Log warning messages to console.

```python
# Log warning
console.warn("This is a warning:", warning_info)
```

#### `info(*messages)`
Log info messages to console.

```python
# Log info
console.info("Information:", info_data)
```

#### `debug(*messages)`
Log debug messages to console.

```python
# Log debug info
console.debug("Debug:", debug_data)
```

#### `clear()`
Clear console.

```python
# Clear console
console.clear()
```

## XMLHttpRequest Object

The `XMLHttpRequest` object enables HTTP requests from Python.

### Properties

| Property | Description | Type |
|----------|-------------|------|
| `ready_state` | Request state | `int` |
| `response` | Response data | `str` |
| `response_text` | Response text | `str` |
| `response_url` | Response URL | `str` |
| `status` | HTTP status code | `int` |
| `status_text` | HTTP status text | `str` |

### Methods

#### `open(method, url, async=True)`
Initialize request.

```python
# Create XMLHttpRequest
xhr = XMLHttpRequest()
xhr.open("GET", "https://api.example.com/data")
```

#### `set_request_header(name, value)`
Set request header.

```python
# Set content type
xhr.set_request_header("Content-Type", "application/json")
```

#### `send(data=None)`
Send request.

```python
# Send GET request
xhr.send()

# Send POST request with data
xhr.send('{"key": "value"}')
```

#### `abort()`
Abort request.

```python
# Abort ongoing request
xhr.abort()
```

### Events

#### `onload`
Called when request completes successfully.

```python
def onload_handler():
    print("Request completed:", xhr.response_text)

xhr.onload = onload_handler
```

#### `onerror`
Called when request fails.

```python
def onerror_handler():
    print("Request failed")

xhr.onerror = onerror_handler
```

#### `onprogress`
Called during request progress.

```python
def onprogress_handler(event):
    print(f"Progress: {event.loaded}/{event.total}")

xhr.onprogress = onprogress_handler
```

## Storage API

Access to localStorage and sessionStorage.

### Properties

None

### Methods

#### `get_item(key)`
Get stored item.

```python
# Get stored value
value = window.localStorage.get_item("my-key")
```

#### `set_item(key, value)`
Set stored item.

```python
# Store value
window.localStorage.set_item("my-key", "my-value")
```

#### `remove_item(key)`
Remove stored item.

```python
# Remove item
window.localStorage.remove_item("my-key")
```

#### `clear()`
Clear all stored items.

```python
# Clear storage
window.localStorage.clear()
```

#### `key(index)`
Get key at index.

```python
# Get first key
first_key = window.localStorage.key(0)
```

#### `length`
Get number of stored items.

```python
# Get storage length
count = window.localStorage.length
```

## JSON Module

Python's built-in JSON module is available for data serialization.

```python
import json

# Parse JSON
data = json.loads('{"key": "value"}')

# Serialize to JSON
json_string = json.dumps({"key": "value"})
```

## Math Module

Python's built-in math module is available for mathematical operations.

```python
import math

# Mathematical functions
result = math.sqrt(16)
angle = math.sin(math.pi / 2)
```

## Datetime Module

Python's datetime module is available for date/time operations.

```python
import datetime

# Current time
now = datetime.datetime.now()

# Date formatting
formatted = now.strftime("%Y-%m-%d %H:%M:%S")
```

## Error Handling

Python exceptions are converted to JavaScript errors and vice versa.

```python
try:
    # DOM operation that might fail
    element = document.find("#nonexistent")
    element.text = "New text"
except AttributeError as e:
    print(f"DOM error: {e}")
except Exception as e:
    print(f"General error: {e}")
```

## Cross-Language Examples

### Calling JavaScript from Python

```python
# Call JavaScript function
result = window.some_js_function("argument")

# Access JavaScript object
js_value = window.js_object.property

# Set JavaScript value
window.js_object.new_property = "Python value"
```

### Calling Python from JavaScript

```javascript
// Call Python function
var result = window.python_function("argument");

// Access Python object
var pyValue = window.python_object.property;

// Set Python value
window.python_object.newProperty = "JavaScript value";
```

## Security Restrictions

Certain operations are restricted for security:

### Forbidden Modules
```python
# These imports will fail
import os          # Security violation
import sys         # Security violation
import subprocess  # Security violation
```

### Restricted File Operations
```python
# These operations will fail
f = open("/etc/passwd", "r")  # Security violation
```

### Network Restrictions
```python
# Cross-origin requests may be blocked
import urllib.request
response = urllib.request.urlopen("http://different-domain.com")  # May be blocked
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

This API reference provides a comprehensive guide to using Python for web development in Ladybird browser, with familiar patterns that Python developers will find intuitive while maintaining the power and flexibility of web APIs.