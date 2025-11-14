#!/usr/bin/env python3
"""
Python Integration Test Script for Ladybird Browser

This script tests core Python functionality that should work
in the Ladybird Python integration environment.
"""

def test_basic_operations():
    """Test basic Python operations"""
    print("🔧 Testing Basic Operations")
    print("-" * 30)
    
    # Arithmetic
    assert 2 + 2 == 4
    print("✓ Addition works")
    
    assert 10 - 3 == 7
    print("✓ Subtraction works")
    
    assert 4 * 5 == 20
    print("✓ Multiplication works")
    
    assert 15 / 3 == 5
    print("✓ Division works")
    
    print("✓ All basic operations passed!\n")


def test_data_structures():
    """Test Python data structures"""
    print("📊 Testing Data Structures")
    print("-" * 30)
    
    # Lists
    numbers = [1, 2, 3, 4, 5]
    assert len(numbers) == 5
    assert sum(numbers) == 15
    print("✓ Lists work")
    
    # List comprehensions
    squares = [x*x for x in numbers]
    assert squares == [1, 4, 9, 16, 25]
    print("✓ List comprehensions work")
    
    # Dictionaries
    person = {"name": "Python", "version": 3.9}
    assert person["name"] == "Python"
    assert len(person) == 2
    print("✓ Dictionaries work")
    
    # Tuples
    coordinates = (10, 20)
    assert coordinates[0] == 10
    assert coordinates[1] == 20
    print("✓ Tuples work")
    
    print("✓ All data structures passed!\n")


def test_control_flow():
    """Test control flow"""
    print("🔄 Testing Control Flow")
    print("-" * 25)
    
    # For loops
    total = 0
    for i in range(5):
        total += i
    assert total == 10
    print("✓ For loops work")
    
    # While loops
    count = 0
    while count < 3:
        count += 1
    assert count == 3
    print("✓ While loops work")
    
    # Conditionals
    if True:
        condition_result = "true"
    else:
        condition_result = "false"
    assert condition_result == "true"
    print("✓ Conditionals work")
    
    print("✓ All control flow passed!\n")


def test_functions():
    """Test function definitions"""
    print("🔧 Testing Functions")
    print("-" * 20)
    
    def add_numbers(a, b):
        return a + b
    
    assert add_numbers(3, 4) == 7
    print("✓ Function definitions work")
    
    # Lambda functions
    multiply = lambda x, y: x * y
    assert multiply(3, 4) == 12
    print("✓ Lambda functions work")
    
    # Recursive functions
    def factorial(n):
        return 1 if n <= 1 else n * factorial(n-1)
    
    assert factorial(5) == 120
    print("✓ Recursive functions work")
    
    print("✓ All function tests passed!\n")


def test_classes():
    """Test object-oriented programming"""
    print("🏗️  Testing Classes")
    print("-" * 18)
    
    class Calculator:
        def __init__(self):
            self.value = 0
        
        def add(self, n):
            self.value += n
            return self
        
        def multiply(self, n):
            self.value *= n
            return self
        
        def get_value(self):
            return self.value
    
    calc = Calculator()
    result = calc.add(5).multiply(3).get_value()
    assert result == 15
    print("✓ Class definitions work")
    print("✓ Method chaining works")
    
    print("✓ All class tests passed!\n")


def test_error_handling():
    """Test exception handling"""
    print("🛡️  Testing Error Handling")
    print("-" * 25)
    
    # Try/except blocks
    try:
        result = 10 / 2
        assert result == 5
        print("✓ Try/except blocks work")
    except ZeroDivisionError:
        print("✗ Unexpected division error")
    
    # Exception raising and catching
    try:
        raise ValueError("Test exception")
    except ValueError as e:
        assert str(e) == "Test exception"
        print("✓ Exception raising/catching works")
    
    print("✓ All error handling tests passed!\n")


def test_string_operations():
    """Test string manipulation"""
    print("📝 Testing String Operations")
    print("-" * 30)
    
    text = "Ladybird Python Integration"
    
    # Basic string operations
    assert len(text) == 27
    print("✓ String length works")
    
    assert text.upper() == "LADYBIRD PYTHON INTEGRATION"
    print("✓ String upper() works")
    
    assert text.lower() == "ladybird python integration"
    print("✓ String lower() works")
    
    # String formatting
    name = "Python"
    version = 3.9
    formatted = f"Hello, {name} {version}!"
    assert formatted == "Hello, Python 3.9!"
    print("✓ String f-formatting works")
    
    print("✓ All string operations passed!\n")


def run_all_tests():
    """Run the complete test suite"""
    print("🐍 Python Integration Test Suite for Ladybird")
    print("=" * 50)
    print()
    
    tests = [
        test_basic_operations,
        test_data_structures,
        test_control_flow,
        test_functions,
        test_classes,
        test_error_handling,
        test_string_operations
    ]
    
    passed = 0
    failed = 0
    
    for test_func in tests:
        try:
            test_func()
            passed += 1
        except Exception as e:
            print(f"✗ {test_func.__name__} FAILED: {e}")
            failed += 1
    
    print("=" * 50)
    print(f"📊 Test Results: {passed} passed, {failed} failed")
    
    if failed == 0:
        print("🎉 ALL TESTS PASSED! Python integration is working correctly.")
    else:
        print("❌ Some tests failed. Check the Python integration setup.")
    
    print("=" * 50)
    
    return failed == 0


if __name__ == "__main__":
    success = run_all_tests()
    exit(0 if success else 1)
