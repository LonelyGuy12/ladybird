# This is an external Python script that can be loaded via src attribute
# It demonstrates the new functionality where Python files can be used externally

def greet(name):
    """Simple greeting function"""
    return f"Hello, {name}! This is from an external Python script."

def calculate_sum(a, b):
    """Simple calculation function"""
    return a + b

# This would run when the script is loaded
print("External Python script loaded successfully!")

# Example usage that would work in a browser context:
# document.find("#output").text = greet("World")