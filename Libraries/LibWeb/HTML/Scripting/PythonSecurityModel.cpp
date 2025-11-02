/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/HashMap.h>
#include <AK/HashTable.h>
#include <AK/Optional.h>
#include <AK/String.h>
#include <AK/StringBuilder.h>
#include <AK/StringUtils.h>
#include <AK/StringView.h>
#include <AK/Vector.h>
#include <LibWeb/HTML/Scripting/PythonSecurityModel.h>
#include <Python.h>
#include <cmath>

namespace Web::HTML {

namespace {

Vector<String> default_safe_builtins()
{
    Vector<String> builtins;
    builtins.ensure_capacity(48);
    auto append = [&](StringView literal) {
        builtins.unchecked_append(MUST(String::from_utf8(literal)));
    };
    append("abs"sv);
    append("all"sv);
    append("any"sv);
    append("bin"sv);
    append("bool"sv);
    append("bytearray"sv);
    append("bytes"sv);
    append("callable"sv);
    append("chr"sv);
    append("complex"sv);
    append("dict"sv);
    append("dir"sv);
    append("divmod"sv);
    append("enumerate"sv);
    append("filter"sv);
    append("float"sv);
    append("format"sv);
    append("frozenset"sv);
    append("hash"sv);
    append("hex"sv);
    append("id"sv);
    append("int"sv);
    append("isinstance"sv);
    append("issubclass"sv);
    append("iter"sv);
    append("len"sv);
    append("list"sv);
    append("map"sv);
    append("max"sv);
    append("min"sv);
    append("next"sv);
    append("object"sv);
    append("oct"sv);
    append("ord"sv);
    append("pow"sv);
    append("range"sv);
    append("repr"sv);
    append("reversed"sv);
    append("round"sv);
    append("set"sv);
    append("slice"sv);
    append("sorted"sv);
    append("str"sv);
    append("sum"sv);
    append("tuple"sv);
    append("type"sv);
    append("zip"sv);
    append("print"sv);
    return builtins;
}

Vector<String> dangerous_patterns()
{
    Vector<String> patterns;
    patterns.ensure_capacity(20);
    auto append = [&](StringView literal) {
        patterns.unchecked_append(MUST(String::from_utf8(literal)));
    };
    append("__import__"sv);
    append("eval("sv);
    append("exec("sv);
    append("globals("sv);
    append("locals("sv);
    append("getattr("sv);
    append("setattr("sv);
    append("delattr("sv);
    append("open("sv);
    append("compile("sv);
    append("input("sv);
    append("subprocess"sv);
    append("ctypes"sv);
    append("os.system"sv);
    append("sys.modules"sv);
    append("importlib"sv);
    append("__class__"sv);
    append("__bases__"sv);
    append("__subclasses__"sv);
    append("builtins.__dict__"sv);
    return patterns;
}

Vector<String> default_allowed_modules()
{
    Vector<String> modules;
    modules.ensure_capacity(15);
    auto append = [&](StringView literal) {
        modules.unchecked_append(MUST(String::from_utf8(literal)));
    };
    append("math"sv);
    append("random"sv);
    append("statistics"sv);
    append("datetime"sv);
    append("json"sv);
    append("collections"sv);
    append("functools"sv);
    append("itertools"sv);
    append("operator"sv);
    append("string"sv);
    append("re"sv);
    append("time"sv);
    append("asyncio"sv);
    append("decimal"sv);
    append("pathlib"sv);
    return modules;
}

String normalize_origin(URL::URL const& origin)
{
    return origin.serialize(URL::ExcludeFragment::Yes);
}

bool host_matches(StringView host, StringView pattern)
{
    if (host == pattern)
        return true;
    if (pattern.starts_with("*."sv)) {
        auto suffix = pattern.substring_view(2);
        return host.length() > suffix.length() && host.ends_with(suffix);
    }
    return false;
}

}

bool PythonSecurityModel::s_security_initialized = false;
HashMap<String, HashTable<String>> PythonSecurityModel::s_origin_allowed_modules;
HashTable<String> PythonSecurityModel::s_safe_domains;
HashMap<String, PythonSecurityModel::ResourceLimits> PythonSecurityModel::s_origin_resource_limits;

ErrorOr<void> PythonSecurityModel::initialize_security()
{
    if (s_security_initialized)
        return {};

    s_safe_domains.ensure_capacity(8);
    s_safe_domains.set("localhost"sv);
    s_safe_domains.set("127.0.0.1"sv);
    s_safe_domains.set("0.0.0.0"sv);

    auto default_modules = default_allowed_modules();
    HashTable<String> module_table;
    for (auto const& module : default_modules)
        module_table.set(module);
    s_origin_allowed_modules.set(MUST(String::from_utf8("default"sv)), move(module_table));

    ResourceLimits default_limits;
    s_origin_resource_limits.set(MUST(String::from_utf8("default"sv)), default_limits);

    s_security_initialized = true;
    return {};
}

ErrorOr<bool> PythonSecurityModel::should_allow_script_execution(String const& script_content, URL::URL const& origin)
{
    TRY(initialize_security());

    if (!TRY(is_code_safe(script_content)))
        return false;

    if (!TRY(check_against_csp(script_content, origin)))
        return false;

    return true;
}

ErrorOr<void> PythonSecurityModel::setup_sandboxed_environment(void* globals_ptr, URL::URL const& origin)
{
    TRY(initialize_security());

    if (!globals_ptr)
        return Error::from_string_literal("Invalid Python globals object");

    auto* globals = static_cast<PyObject*>(globals_ptr);
    TRY(restrict_builtins(globals));

    auto limits = s_origin_resource_limits.get(normalize_origin(origin)).value_or(s_origin_resource_limits.get(MUST(String::from_utf8("default"sv))).value());
    TRY(set_resource_limits(globals_ptr, limits));

    return {};
}
ErrorOr<void> PythonSecurityModel::restrict_builtins(void* globals_ptr)
{
    if (!globals_ptr)
        return Error::from_string_literal("Invalid Python globals");

    auto* globals = static_cast<PyObject*>(globals_ptr);
    PyObject* builtins_module = PyEval_GetBuiltins();
    if (!builtins_module)
        return Error::from_string_literal("Failed to retrieve Python builtins");

    PyObject* safe_builtins = PyDict_New();
    if (!safe_builtins)
        return Error::from_string_literal("Failed to create safe builtins dictionary"sv);

    auto safe_names = default_safe_builtins();
    for (auto const& name : safe_names) {
        auto name_bytes = MUST(name.to_byte_string());
        PyObject* builtin = PyDict_GetItemString(builtins_module, name_bytes.characters());
        if (builtin)
            PyDict_SetItemString(safe_builtins, name_bytes.characters(), builtin);
    }

    PyDict_SetItemString(globals, "__builtins__", safe_builtins);
    Py_DECREF(safe_builtins);
    return {};
}

ErrorOr<bool> PythonSecurityModel::should_allow_module_import(String const& module_name, URL::URL const& origin)
{
    TRY(initialize_security());

    auto modules = get_allowed_modules(origin);
    for (auto const& allowed : modules) {
        if (module_name == allowed)
            return true;
        if (module_name.starts_with(allowed.bytes_as_string_view()) && module_name[allowed.length()] == '.')
            return true;
    }

    return false;
}

ErrorOr<void> PythonSecurityModel::setup_restricted_filesystem_access(void* interpreter)
{
    if (!interpreter)
        return Error::from_string_literal("Invalid interpreter state");

    PyObject* globals = static_cast<PyObject*>(interpreter);
    PyDict_DelItemString(globals, "open");

    PyObject* os_module = PyImport_ImportModule("os");
    if (os_module) {
        PyObject* os_disabled = PyModule_New("os_disabled");
        if (os_disabled)
            PyDict_SetItemString(globals, "os", os_disabled);
        Py_XDECREF(os_disabled);
        Py_DECREF(os_module);
    }

    PyObject* sys_module = PyImport_ImportModule("sys");
    if (sys_module) {
        PyObject* modules_dict = PyObject_GetAttrString(sys_module, "modules");
        if (modules_dict)
            PyDict_SetItemString(modules_dict, "os", Py_None);
        Py_XDECREF(modules_dict);
        Py_DECREF(sys_module);
    }

    return {};
}

ErrorOr<bool> PythonSecurityModel::should_allow_network_request(URL::URL const& target_url, URL::URL const& origin)
{
    if (!TRY(check_if_same_origin(target_url, origin))) {
        auto host_string = target_url.host().has_value() ? MUST(target_url.host()->serialize()).bytes_as_string_view() : StringView {};
        bool allowed = false;
        for (auto const& domain : s_safe_domains) {
            if (host_matches(host_string, domain.bytes_as_string_view())) {
                allowed = true;
                break;
            }
        }
        if (!allowed)
            return false;
    }

    return true;
}

ErrorOr<void> PythonSecurityModel::set_resource_limits(void* interpreter, ResourceLimits const& limits)
{
    if (!interpreter)
        return Error::from_string_literal("Invalid interpreter");

    TRY(setup_memory_limiter(interpreter, limits.max_memory_bytes));
    TRY(setup_cpu_limiter(interpreter, limits.max_cpu_time_ms));

    auto recursion_command = MUST(String::formatted("import sys; sys.setrecursionlimit({})", limits.max_recursion_depth));
    auto recursion_c_string = MUST(recursion_command.to_byte_string());
    if (PyRun_SimpleString(recursion_c_string.characters()) != 0)
        return Error::from_string_literal("Failed to set recursion limit"sv);

    return {};
}

ErrorOr<void> PythonSecurityModel::add_safe_domain(String const& domain)
{
    s_safe_domains.set(domain);
    return {};
}

ErrorOr<bool> PythonSecurityModel::is_code_safe(String const& code)
{
    for (auto const& pattern : dangerous_patterns()) {
        if (code.contains(pattern.bytes_as_string_view(), CaseSensitivity::CaseSensitive))
            return false;
    }
    return true;
}

ErrorOr<bool> PythonSecurityModel::check_against_csp(String const&, URL::URL const&)
{
    return true;
}

Vector<String> PythonSecurityModel::get_allowed_modules(URL::URL const& origin)
{
    auto origin_key = normalize_origin(origin);
    if (auto entry = s_origin_allowed_modules.get(origin_key); entry.has_value()) {
        Vector<String> modules;
        modules.ensure_capacity(entry->size());
        for (auto const& module : *entry)
            modules.append(module);
        return modules;
    }

    Vector<String> modules;
    if (auto default_entry = s_origin_allowed_modules.get("default"_string); default_entry.has_value()) {
        modules.ensure_capacity(default_entry->size());
        for (auto const& module : *default_entry)
            modules.append(module);
    }
    return modules;
}

HashMap<String, double> PythonSecurityModel::get_resource_usage(void*)
{
    HashMap<String, double> usage;
    usage.set("cpu_time_ms", 0.0);
    usage.set("memory_bytes", 0.0);
    usage.set("executions", 0.0);
    return usage;
}

ErrorOr<bool> PythonSecurityModel::check_code_for_dangerous_patterns(String const& code)
{
    return is_code_safe(code);
}

ErrorOr<bool> PythonSecurityModel::check_if_same_origin(URL::URL const& url1, URL::URL const& url2)
{
    return url1.origin() == url2.origin();
}

ErrorOr<void> PythonSecurityModel::setup_memory_limiter(void* interpreter, u32 max_bytes)
{
#if defined(AK_OS_UNIX)
    String command = MUST(String::formatted(
        "import resource; resource.setrlimit(resource.RLIMIT_AS, ({}, {}))", max_bytes, max_bytes));
    auto command_c_str = MUST(command.to_byte_string());
    if (PyRun_SimpleString(command_c_str.characters()) != 0)
        return Error::from_string_literal("Failed to set memory limit"sv);
#else
    (void)interpreter;
    (void)max_bytes;
#endif
    return {};
}

ErrorOr<void> PythonSecurityModel::setup_cpu_limiter(void*, u32 max_ms)
{
#if defined(AK_OS_UNIX)
    double seconds = static_cast<double>(max_ms) / 1000.0;
    auto rounded = static_cast<unsigned>(std::ceil(seconds));
    String command = MUST(String::formatted(
        "import resource, signal, threading; "
        "resource.setrlimit(resource.RLIMIT_CPU, ({}, {})); "
        "signal.alarm({})",
        seconds, seconds, rounded));
    auto command_c_str = MUST(command.to_byte_string());
    if (PyRun_SimpleString(command_c_str.characters()) != 0)
        return Error::from_string_literal("Failed to configure CPU limiter"sv);
#else
    (void)max_ms;
#endif
    return {};
}

} // namespace Web::HTML
