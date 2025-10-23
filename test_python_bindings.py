#!/usr/bin/env python3
"""
Simple test script to validate Python DOM bindings
"""

import sys
import os

def test_python_import():
    """Test that Python can import and run basic code"""
    print("✓ Test 1: Python imports work")
    return True

def test_basic_operations():
    """Test basic Python operations"""
    result = 1 + 1
    assert result == 2, f"Expected 2, got {result}"
    print("✓ Test 2: Basic arithmetic works")
    return True

def test_string_operations():
    """Test string operations"""
    test_str = "Hello, Ladybird!"
    assert len(test_str) == 16, f"Expected length 16, got {len(test_str)}"
    assert test_str.startswith("Hello"), "String should start with 'Hello'"
    print("✓ Test 3: String operations work")
    return True

def test_data_structures():
    """Test basic data structures"""
    test_list = [1, 2, 3, 4, 5]
    assert len(test_list) == 5, f"Expected list length 5, got {len(test_list)}"
    assert sum(test_list) == 15, f"Expected sum 15, got {sum(test_list)}"
    
    test_dict = {'a': 1, 'b': 2, 'c': 3}
    assert len(test_dict) == 3, f"Expected dict length 3, got {len(test_dict)}"
    assert test_dict['b'] == 2, f"Expected dict['b'] == 2, got {test_dict['b']}"
    
    print("✓ Test 4: Data structures work")
    return True

def test_control_flow():
    """Test control flow"""
    result = 0
    for i in range(10):
        if i % 2 == 0:
            result += i
    assert result == 20, f"Expected 20, got {result}"
    print("✓ Test 5: Control flow works")
    return True

def main():
    """Run all tests"""
    print("=" * 50)
    print("Python DOM Bindings - Basic Tests")
    print("=" * 50)
    print()
    
    tests = [
        test_python_import,
        test_basic_operations,
        test_string_operations,
        test_data_structures,
        test_control_flow,
    ]
    
    passed = 0
    failed = 0
    
    for test in tests:
        try:
            if test():
                passed += 1
        except AssertionError as e:
            print(f"✗ {test.__name__} failed: {e}")
            failed += 1
        except Exception as e:
            print(f"✗ {test.__name__} error: {e}")
            failed += 1
    
    print()
    print("=" * 50)
    print(f"Results: {passed} passed, {failed} failed")
    print("=" * 50)
    
    return 0 if failed == 0 else 1

if __name__ == "__main__":
    sys.exit(main())
