/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibTest/TestCase.h>
#include <LibWeb/HTML/Scripting/IndependentPythonEngine.h>
#include <LibWeb/HTML/Scripting/PythonSecurityModel.h>
#include <LibWeb/HTML/Scripting/PythonStdLib.h>
#include <LibWeb/Bindings/OptionalPythonJSBridge.h>
#include <LibJS/Runtime/VM.h>
#include <LibJS/Runtime/Value.h>
#include <LibURL/URL.h>

namespace Web::Tests {

TEST_CASE(test_python_engine_initialization)
{
    auto result = HTML::IndependentPythonEngine::create();
    EXPECT(!result.is_error());
    
    auto& engine = result.value();
    EXPECT(engine->is_initialized());
    
    auto version = engine->version();
    EXPECT(!version.is_empty());
}

TEST_CASE(test_python_code_execution)
{
    auto engine_result = HTML::IndependentPythonEngine::create();
    EXPECT(!engine_result.is_error());
    auto& engine = engine_result.value();
    
    // Execute simple Python code
    auto result = engine->run("1 + 1");
    EXPECT(!result.is_error());
    
    // Check result
    EXPECT(result.value().is_number());
    EXPECT_EQ(result.value().as_double(), 2.0);
}

TEST_CASE(test_python_module_import)
{
    auto engine_result = HTML::IndependentPythonEngine::create();
    EXPECT(!engine_result.is_error());
    auto& engine = engine_result.value();
    
    // Import a standard module
    auto result = engine->run("import math; math.pi");
    EXPECT(!result.is_error());
    
    // Check result
    EXPECT(result.value().is_number());
    EXPECT_EQ(result.value().as_double(), 3.14159265358979);
}

TEST_CASE(test_python_security_model)
{
    auto result = HTML::PythonSecurityModel::initialize_security();
    EXPECT(!result.is_error());
    
    URL::URL origin("https://example.com"_string);
    
    // Test if module import is allowed
    auto import_allowed = HTML::PythonSecurityModel::should_allow_module_import("json", origin);
    EXPECT(!import_allowed.is_error());
    EXPECT(import_allowed.value() == true);
    
    // Test if dangerous module import is blocked
    auto dangerous_import = HTML::PythonSecurityModel::should_allow_module_import("os", origin);
    EXPECT(!dangerous_import.is_error());
    EXPECT(dangerous_import.value() == false);
    
    // Test code safety check
    auto safe_code = HTML::PythonSecurityModel::is_code_safe("print('Hello, world!')");
    EXPECT(!safe_code.is_error());
    EXPECT(safe_code.value() == true);
    
    // Test dangerous code detection
    auto dangerous_code = HTML::PythonSecurityModel::is_code_safe("__import__('os').system('rm -rf /')");
    EXPECT(!dangerous_code.is_error());
    EXPECT(dangerous_code.value() == false);
}

TEST_CASE(test_python_stdlib)
{
    auto stdlib_result = HTML::Python::PythonStdLib::create();
    EXPECT(!stdlib_result.is_error());
    auto& stdlib = stdlib_result.value();
    
    // Check JSON module
    auto json_module = stdlib->get_module("json");
    EXPECT(json_module != nullptr);
    
    // Check DOM module
    auto dom_module = stdlib->get_module("dom");
    EXPECT(dom_module != nullptr);
}

TEST_CASE(test_optional_js_bridge)
{
    auto bridge_result = Bindings::OptionalPythonJSBridge::create();
    EXPECT(!bridge_result.is_error());
    auto& bridge = bridge_result.value();
    
    // Check initialization
    EXPECT(bridge->is_initialized());
    
    // Check initial stats
    auto stats = bridge->get_stats();
    EXPECT_EQ(stats.messages_to_js, 0U);
    EXPECT_EQ(stats.messages_to_python, 0U);
}

TEST_CASE(test_python_performance)
{
    auto engine_result = HTML::IndependentPythonEngine::create();
    EXPECT(!engine_result.is_error());
    auto& engine = engine_result.value();
    
    // Execute complex Python code for performance measurement
    const char* performance_test = R"(
sum = 0
for i in range(1000000):
    sum += i
sum
)";
    
    auto result = engine->run(performance_test);
    EXPECT(!result.is_error());
    
    // Check performance metrics
    auto stats = engine->get_performance_stats();
    EXPECT(stats.execution_time_ns > 0);
    EXPECT(stats.function_calls > 0);
}

TEST_CASE(test_python_async_support)
{
    auto engine_result = HTML::IndependentPythonEngine::create();
    EXPECT(!engine_result.is_error());
    auto& engine = engine_result.value();
    
    // Test async Python code
    const char* async_test = R"(
import asyncio

async def hello():
    return "Hello, async world!"

asyncio.run(hello())
)";
    
    auto result = engine->run(async_test);
    EXPECT(!result.is_error());
    EXPECT(result.value().is_string());
}

TEST_CASE(test_python_dom_integration)
{
    auto engine_result = HTML::IndependentPythonEngine::create();
    EXPECT(!engine_result.is_error());
    auto& engine = engine_result.value();
    
    // Test DOM integration
    const char* dom_test = R"(
import dom

# Creating a new DOM element should succeed but return null in test environment
element = dom.create_element('div')
element is None
)";
    
    auto result = engine->run(dom_test);
    EXPECT(!result.is_error());
    EXPECT(result.value().is_boolean());
    EXPECT(result.value().as_bool() == true);
}

} // namespace Web::Tests
