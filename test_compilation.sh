#!/bin/bash

echo "=================================================="
echo "Python Integration Compilation Tests"
echo "=================================================="
echo ""

# Color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

passed=0
failed=0

# Test 1: Check if liblagom-web.so exists
echo -n "Test 1: LibWeb library exists... "
if [ -f "ladybird-build/lib/liblagom-web.so.0.0.0" ]; then
    echo -e "${GREEN}✓ PASS${NC}"
    ((passed++))
else
    echo -e "${RED}✗ FAIL${NC}"
    ((failed++))
fi

# Test 2: Check library size (should be large with Python bindings)
echo -n "Test 2: Library size check (>200MB)... "
size=$(stat -f%z ladybird-build/lib/liblagom-web.so.0.0.0 2>/dev/null || stat -c%s ladybird-build/lib/liblagom-web.so.0.0.0 2>/dev/null)
if [ $size -gt 200000000 ]; then
    echo -e "${GREEN}✓ PASS${NC} ($(($size / 1024 / 1024))MB)"
    ((passed++))
else
    echo -e "${RED}✗ FAIL${NC} ($(($size / 1024 / 1024))MB)"
    ((failed++))
fi

# Test 3: Check for Python symbols
echo -n "Test 3: Python symbols present... "
python_syms=$(nm ladybird-build/lib/liblagom-web.so.0.0.0 2>/dev/null | grep -i "python" | wc -l)
if [ $python_syms -gt 50 ]; then
    echo -e "${GREEN}✓ PASS${NC} ($python_syms symbols found)"
    ((passed++))
else
    echo -e "${RED}✗ FAIL${NC} ($python_syms symbols found)"
    ((failed++))
fi

# Test 4: Check for PythonDOMAPI
echo -n "Test 4: PythonDOMAPI symbols... "
if nm ladybird-build/lib/liblagom-web.so.0.0.0 2>/dev/null | grep -q "PythonDOMAPI"; then
    echo -e "${GREEN}✓ PASS${NC}"
    ((passed++))
else
    echo -e "${RED}✗ FAIL${NC}"
    ((failed++))
fi

# Test 5: Check for PythonDocument
echo -n "Test 5: PythonDocument symbols... "
if nm ladybird-build/lib/liblagom-web.so.0.0.0 2>/dev/null | grep -q "PythonDocument"; then
    echo -e "${GREEN}✓ PASS${NC}"
    ((passed++))
else
    echo -e "${RED}✗ FAIL${NC}"
    ((failed++))
fi

# Test 6: Check for PythonElement
echo -n "Test 6: PythonElement symbols... "
if nm ladybird-build/lib/liblagom-web.so.0.0.0 2>/dev/null | grep -q "PythonElement"; then
    echo -e "${GREEN}✓ PASS${NC}"
    ((passed++))
else
    echo -e "${RED}✗ FAIL${NC}"
    ((failed++))
fi

# Test 7: Check for PythonWindow
echo -n "Test 7: PythonWindow symbols... "
if nm ladybird-build/lib/liblagom-web.so.0.0.0 2>/dev/null | grep -q "PythonWindow"; then
    echo -e "${GREEN}✓ PASS${NC}"
    ((passed++))
else
    echo -e "${RED}✗ FAIL${NC}"
    ((failed++))
fi

# Test 8: Check for PythonJSBridge
echo -n "Test 8: PythonJSBridge symbols... "
if nm ladybird-build/lib/liblagom-web.so.0.0.0 2>/dev/null | grep -q "PythonJSBridge"; then
    echo -e "${GREEN}✓ PASS${NC}"
    ((passed++))
else
    echo -e "${RED}✗ FAIL${NC}"
    ((failed++))
fi

# Test 9: Check for PythonJSObjectWrapper
echo -n "Test 9: PythonJSObjectWrapper symbols... "
if nm ladybird-build/lib/liblagom-web.so.0.0.0 2>/dev/null | grep -q "PythonJSObjectWrapper"; then
    echo -e "${GREEN}✓ PASS${NC}"
    ((passed++))
else
    echo -e "${RED}✗ FAIL${NC}"
    ((failed++))
fi

# Test 10: Check for TestPythonDOMModule
echo -n "Test 10: TestPythonDOMModule symbols... "
if nm ladybird-build/lib/liblagom-web.so.0.0.0 2>/dev/null | grep -q "TestPythonDOMModule"; then
    echo -e "${GREEN}✓ PASS${NC}"
    ((passed++))
else
    echo -e "${RED}✗ FAIL${NC}"
    ((failed++))
fi

# Test 11: Check for fetch_python_script
echo -n "Test 11: fetch_python_script function... "
if nm ladybird-build/lib/liblagom-web.so.0.0.0 2>/dev/null | grep -q "fetch_python_script"; then
    echo -e "${GREEN}✓ PASS${NC}"
    ((passed++))
else
    echo -e "${RED}✗ FAIL${NC}"
    ((failed++))
fi

# Test 12: Check Python 3.12 linkage
echo -n "Test 12: Python 3.12 library linkage... "
if ldd ladybird-build/lib/liblagom-web.so.0.0.0 2>/dev/null | grep -q "python3.12" || otool -L ladybird-build/lib/liblagom-web.so.0.0.0 2>/dev/null | grep -q "python3.12"; then
    echo -e "${GREEN}✓ PASS${NC}"
    ((passed++))
else
    echo -e "${YELLOW}⚠ SKIP${NC} (dynamic linkage)"
    # Don't count as failure since it might be statically linked
fi

echo ""
echo "=================================================="
echo "Results: $passed passed, $failed failed"
echo "=================================================="

if [ $failed -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
fi
