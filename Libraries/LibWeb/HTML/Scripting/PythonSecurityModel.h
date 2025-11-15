/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/HashMap.h>
#include <AK/String.h>
#include <AK/HashTable.h>
#include <LibURL/Origin.h>
#include <LibURL/URL.h>
#include <LibWeb/Forward.h>

namespace Web::HTML {

// Independent Python security model with no JS dependencies
class PythonSecurityModel {
public:
    // Initialize the security model
    static ErrorOr<void> initialize_security();

    // Set up restricted builtins in the global Python builtins module
    // This should be called during Python initialization, before any scripts run
    static ErrorOr<void> setup_global_restricted_builtins();

    // Check if a Python script should be allowed to run
    static ErrorOr<bool> should_allow_script_execution(String const& script_content, URL::URL const& origin);

    // Set up sandboxed execution environment for Python
    static ErrorOr<void> setup_sandboxed_environment(void* globals, URL::URL const& origin);

    // Restrict Python built-ins to safe operations only
    static ErrorOr<void> restrict_builtins(void* globals);

    // Check if a module import should be allowed
    static ErrorOr<bool> should_allow_module_import(String const& module_name, URL::URL const& origin);

    // Set up restricted file system access
    static ErrorOr<void> setup_restricted_filesystem_access(void* interpreter);

    // Check if a network request should be allowed
    static ErrorOr<bool> should_allow_network_request(URL::URL const& target_url, URL::URL const& origin);

    // Resource usage limits for Python execution
    struct ResourceLimits {
        u32 max_memory_bytes { 100 * 1024 * 1024 }; // 100MB
        u32 max_cpu_time_ms { 5000 }; // 5 seconds
        u32 max_stack_depth { 1000 };
        u32 max_recursion_depth { 100 };
        u32 max_module_imports { 50 };
    };

    // Set resource limits for Python execution
    static ErrorOr<void> set_resource_limits(void* interpreter, ResourceLimits const& limits);

    // Add a safe domain for cross-origin requests
    static ErrorOr<void> add_safe_domain(String const& domain);

    // Check if code contains potentially dangerous patterns
    static ErrorOr<bool> is_code_safe(String const& code);

    // Content Security Policy integration
    static ErrorOr<bool> check_against_csp(String const& code, URL::URL const& origin);

    // Get a list of allowed modules for the given origin
    static Vector<String> get_allowed_modules(URL::URL const& origin);

    // Get resource usage statistics
    static HashMap<String, double> get_resource_usage(void* interpreter);

private:
    static bool s_security_initialized;
    static HashMap<String, HashTable<String>> s_origin_allowed_modules;
    static HashTable<String> s_safe_domains;
    static HashMap<String, ResourceLimits> s_origin_resource_limits;

    // Helper methods
    static ErrorOr<bool> check_code_for_dangerous_patterns(String const& code);
    static ErrorOr<bool> check_if_same_origin(URL::URL const& url1, URL::URL const& url2);
    static ErrorOr<void> setup_memory_limiter(void* interpreter, u32 max_bytes);
    static ErrorOr<void> setup_cpu_limiter(void* interpreter, u32 max_ms);
};

} // namespace Web::HTML
