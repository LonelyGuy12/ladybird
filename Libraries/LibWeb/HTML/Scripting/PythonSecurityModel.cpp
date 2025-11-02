/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/HashTable.h>
#include <AK/HashMap.h>
#include <AK/String.h>
#include <AK/StringBuilder.h>
#include <AK/Vector.h>
#include <AK/Time.h>
#include <LibRegex/RegexMatcher.h>
#include <LibURL/Origin.h>
#include <LibURL/URL.h>
#include <LibWeb/HTML/Scripting/PythonSecurityModel.h>

namespace Web::HTML {

bool PythonSecurityModel::s_security_initialized = false;
HashMap<String, HashTable<String>> PythonSecurityModel::s_origin_allowed_modules;
HashTable<String> PythonSecurityModel::s_safe_domains;
HashMap<String, PythonSecurityModel::ResourceLimits> PythonSecurityModel::s_origin_resource_limits;

// Independent security model implementation with no JS dependencies
ErrorOr<void> PythonSecurityModel::initialize_security()
{
    if (s_security_initialized)
        return {};

    // Initialize safe domains list
    s_safe_domains.set("api.example.com");
    s_safe_domains.set("cdn.example.com");
    s_safe_domains.set("fonts.googleapis.com");
    s_safe_domains.set("cdn.jsdelivr.net");

    // Setup default allowed modules by origin
    HashTable<String> default_modules;
    default_modules.set("json");
    default_modules.set("datetime");
    default_modules.set("math");
    default_modules.set("random");
    default_modules.set("re");
    default_modules.set("collections");
    default_modules.set("itertools");
    default_modules.set("functools");
    default_modules.set("hashlib");
    default_modules.set("base64");
    default_modules.set("string");
    
    // Special modules for trusted origins
    HashTable<String> trusted_modules = default_modules;
    trusted_modules.set("asyncio");
    trusted_modules.set("time");
    trusted_modules.set("threading");
    
    // Browser-specific modules
    trusted_modules.set("browser");
    trusted_modules.set("dom");
    trusted_modules.set("webapi");
    
    // Set default modules for all origins
    s_origin_allowed_modules.set("default", move(default_modules));
    
    // Set trusted modules for specific origins
    s_origin_allowed_modules.set("https://trusted.example.com", move(trusted_modules));
    
    // Set default resource limits
    ResourceLimits default_limits;
    s_origin_resource_limits.set("default", default_limits);
    
    // Set higher limits for trusted origins
    ResourceLimits trusted_limits;
    trusted_limits.max_memory_bytes = 200 * 1024 * 1024; // 200MB
    trusted_limits.max_cpu_time_ms = 10000; // 10 seconds
    trusted_limits.max_recursion_depth = 200;
    s_origin_resource_limits.set("https://trusted.example.com", trusted_limits);
    
    s_security_initialized = true;
    return {};
}

ErrorOr<bool> PythonSecurityModel::should_allow_script_execution(String const& script_content, URL::URL const& origin)
{
    // Check against CSP
    TRY(check_against_csp(script_content, origin));
    
    // Check for dangerous patterns in code
    TRY(is_code_safe(script_content));
    
    // Check if origin is allowed to run Python scripts
    // In a real implementation, we would check against the browser's security policies
    
    return true;
}

ErrorOr<void> PythonSecurityModel::setup_sandboxed_environment(void* globals_ptr, URL::URL const& origin)
{
    if (!globals_ptr)
        return Error::from_string_literal("Invalid globals dictionary");
        
    // Cast void* to appropriate type at implementation level
    auto* globals = static_cast<void*>(globals_ptr);
    
    // Start with safe built-ins only
    TRY(restrict_builtins(globals));
    
    // Get resource limits for the origin
    auto& limits = s_origin_resource_limits.contains(origin.to_string()) ?
        s_origin_resource_limits.get(origin.to_string()).value() :
        s_origin_resource_limits.get("default").value();
    
    // Set up memory and CPU usage limits
    TRY(setup_memory_limiter(globals, limits.max_memory_bytes));
    TRY(setup_cpu_limiter(globals, limits.max_cpu_time_ms));
    
    // Set up recursion limit
    // In actual implementation, call PyRun_SimpleString("sys.setrecursionlimit(limit)")
    
    return {};
}

ErrorOr<void> PythonSecurityModel::restrict_builtins(void* globals_ptr)
{
    if (!globals_ptr)
        return Error::from_string_literal("Invalid globals dictionary");
    
    // Implementation details at C API level
    // Here we just define the security policy
    
    // Safe built-in functions that should be allowed
    Vector<String> safe_builtins = {
        "abs", "all", "any", "bool", "chr", "dict", "dir", "divmod", "enumerate", "filter", "float",
        "format", "frozenset", "hasattr", "hash", "hex", "id", "int", "isinstance", "issubclass", 
        "iter", "len", "list", "map", "max", "min", "next", "object", "oct", "ord", "pow",
        "range", "repr", "reversed", "round", "set", "slice", "sorted", "str", "sum", "tuple", 
        "type", "zip", "True", "False", "None"
    };
    
    // Block dangerous built-ins
    Vector<String> dangerous_builtins = {
        "__import__", "compile", "eval", "exec", "globals", "locals", "open", 
        "input", "breakpoint", "memoryview", "vars", "getattr", "setattr", 
        "delattr", "classmethod", "staticmethod", "property"
    };
    
    // In a real implementation, we would iterate through builtins and only
    // allow those in the safe_builtins list
    
    return {};
}

ErrorOr<bool> PythonSecurityModel::should_allow_module_import(String const& module_name, URL::URL const& origin)
{
    // Get allowed modules for the origin
    auto allowed_modules = get_allowed_modules(origin);
    
    // Check if module is in the allowed list
    for (auto& allowed : allowed_modules) {
        if (allowed == module_name)
            return true;
    }
    
    // Check if this is a submodule of an allowed module
    // e.g. if json is allowed, json.decoder should also be allowed
    for (auto& allowed : allowed_modules) {
        if (module_name.starts_with(allowed + "."))
            return true;
    }
    
    return false;
}

ErrorOr<void> PythonSecurityModel::setup_restricted_filesystem_access(void* interpreter)
{
    if (!interpreter)
        return Error::from_string_literal("Invalid interpreter");
    
    // Override Python's open(), file objects, etc. with sandboxed versions
    // In a real implementation, we would inject a custom __builtins__ with a restricted open()
    // that only allows access to a virtual filesystem
    
    // 1. Replace built-in open() function
    // 2. Override os.* filesystem functions
    // 3. Set up a virtual filesystem for allowed operations
    
    return {};
}

ErrorOr<bool> PythonSecurityModel::should_allow_network_request(URL::URL const& target_url, URL::URL const& origin)
{
    // 1. Same-origin check
    if (TRY(check_if_same_origin(target_url, origin)))
        return true;
    
    // 2. Check against safe domains list
    if (s_safe_domains.contains(target_url.host()))
        return true;
        
    // 3. Check if target is in a subdomain of an allowed domain
    for (auto& safe_domain : s_safe_domains) {
        if (target_url.host().ends_with("." + safe_domain))
            return true;
    }
    
    // 4. In a real implementation, we would also check CORS headers
    
    return false;
}

ErrorOr<void> PythonSecurityModel::set_resource_limits(void* interpreter, ResourceLimits const& limits)
{
    if (!interpreter)
        return Error::from_string_literal("Invalid interpreter");
        
    // Set memory limit
    TRY(setup_memory_limiter(interpreter, limits.max_memory_bytes));
    
    // Set CPU time limit
    TRY(setup_cpu_limiter(interpreter, limits.max_cpu_time_ms));
    
    // Set recursion depth
    // PyRun_SimpleString("import sys; sys.setrecursionlimit(limits.max_recursion_depth)")
    
    // Set stack depth limit
    // This would be implementation-specific
    
    return {};
}

ErrorOr<void> PythonSecurityModel::add_safe_domain(String const& domain)
{
    s_safe_domains.set(domain);
    return {};
}

ErrorOr<bool> PythonSecurityModel::is_code_safe(String const& code)
{
    return TRY(check_code_for_dangerous_patterns(code));
}

ErrorOr<bool> PythonSecurityModel::check_against_csp(String const& code, URL::URL const& origin)
{
    // This would integrate with the browser's Content Security Policy system
    // For now, we assume all Python code is allowed if the origin is trusted
    
    // In a real implementation, we would check if the CSP allows 'unsafe-eval'
    // or if we have a specific directive for Python execution
    
    return true;
}

Vector<String> PythonSecurityModel::get_allowed_modules(URL::URL const& origin)
{
    Vector<String> result;
    
    // Check if we have specific modules for this origin
    auto origin_str = origin.to_string();
    if (s_origin_allowed_modules.contains(origin_str)) {
        for (auto& module : s_origin_allowed_modules.get(origin_str).value())
            result.append(module);
    } else {
        // Use default modules
        for (auto& module : s_origin_allowed_modules.get("default").value())
            result.append(module);
    }
    
    return result;
}

HashMap<String, double> PythonSecurityModel::get_resource_usage(void* interpreter)
{
    HashMap<String, double> result;
    
    // In a real implementation, we would query the interpreter for:
    // - Memory usage
    // - CPU time used
    // - Number of operations executed
    // - Number of module imports
    // - Stack depth
    
    result.set("memory_bytes", 0.0); // Placeholder
    result.set("cpu_time_ms", 0.0); // Placeholder
    result.set("operations_count", 0.0); // Placeholder
    
    return result;
}

// Helper methods
ErrorOr<bool> PythonSecurityModel::check_code_for_dangerous_patterns(String const& code)
{
    // Check for dangerous patterns in Python code
    // This is a basic implementation that would be expanded in a real system
    
    Vector<StringView> dangerous_patterns = {
        "__import__(", 
        "eval(",
        "exec(",
        "globals(",
        "locals(",
        "getattr(",
        "setattr(",
        "delattr(",
        "open(",
        "__builtins__",
        "__base__",
        "__mro__",
        "__subclasses__",
        "sys._getframe",
        "ctypes"
    };
    
    for (auto& pattern : dangerous_patterns) {
        if (code.contains(pattern))
            return false;
    }
    
    return true;
}

ErrorOr<bool> PythonSecurityModel::check_if_same_origin(URL::URL const& url1, URL::URL const& url2)
{
    URL::Origin origin1 = url1.origin();
    URL::Origin origin2 = url2.origin();
    
    return origin1 == origin2;
}

ErrorOr<void> PythonSecurityModel::setup_memory_limiter(void* interpreter, u32 max_bytes)
{
    // In a real implementation, we would use Python's resource module or
    // a custom memory allocator to limit memory usage
    
    // For example:
    // PyRun_SimpleString("import resource; resource.setrlimit(resource.RLIMIT_AS, (max_bytes, max_bytes))");
    
    return {};
}

ErrorOr<void> PythonSecurityModel::setup_cpu_limiter(void* interpreter, u32 max_ms)
{
    // In a real implementation, we would set up signal handlers to interrupt
    // Python execution if it takes too long
    
    // For example:
    // PyRun_SimpleString("import signal; signal.setitimer(signal.ITIMER_VIRTUAL, max_ms / 1000.0)")
    
    return {};
}

} // namespace Web::HTML
