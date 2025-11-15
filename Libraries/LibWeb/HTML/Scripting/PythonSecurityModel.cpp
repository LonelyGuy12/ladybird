/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Array.h>
#include <AK/HashMap.h>
#include <AK/HashTable.h>
#include <AK/Optional.h>
#include <AK/String.h>
#include <AK/StringBuilder.h>
#include <AK/StringUtils.h>
#include <AK/StringView.h>
#include <AK/Try.h>
#include <AK/Vector.h>
#include <LibWeb/HTML/Scripting/PythonSecurityModel.h>
#include <Python.h>
#include <cmath>

namespace Web::HTML {

namespace {

static String const& default_origin_key()
{
    static String key = MUST(String::from_utf8("default"sv));
    return key;
}

Vector<String> default_safe_builtins()
{
    Vector<String> builtins;
    builtins.ensure_capacity(80);
    auto append_literal = [&](StringView literal) {
        builtins.unchecked_append(MUST(String::from_utf8(literal)));
    };

    // Core language/runtime support
    append_literal("__build_class__"sv); // required for class statements
    append_literal("__import__"sv);      // required for import statements
    append_literal("object"sv);
    append_literal("type"sv);
    append_literal("super"sv);

    // Common exceptions needed for try/except
    append_literal("BaseException"sv);
    append_literal("Exception"sv);
    append_literal("TypeError"sv);
    append_literal("ValueError"sv);
    append_literal("ZeroDivisionError"sv);

    // Introspection and data model helpers (safe subset)
    append_literal("abs"sv);
    append_literal("all"sv);
    append_literal("any"sv);
    append_literal("bin"sv);
    append_literal("bool"sv);
    append_literal("bytearray"sv);
    append_literal("bytes"sv);
    append_literal("callable"sv);
    append_literal("chr"sv);
    append_literal("complex"sv);
    append_literal("dict"sv);
    append_literal("dir"sv);
    append_literal("divmod"sv);
    append_literal("enumerate"sv);
    append_literal("filter"sv);
    append_literal("float"sv);
    append_literal("format"sv);
    append_literal("frozenset"sv);
    append_literal("hash"sv);
    append_literal("hex"sv);
    append_literal("id"sv);
    append_literal("int"sv);
    append_literal("isinstance"sv);
    append_literal("issubclass"sv);
    append_literal("iter"sv);
    append_literal("len"sv);
    append_literal("list"sv);
    append_literal("map"sv);
    append_literal("max"sv);
    append_literal("min"sv);
    append_literal("next"sv);
    append_literal("oct"sv);
    append_literal("ord"sv);
    append_literal("pow"sv);
    append_literal("range"sv);
    append_literal("repr"sv);
    append_literal("reversed"sv);
    append_literal("round"sv);
    append_literal("set"sv);
    append_literal("slice"sv);
    append_literal("sorted"sv);
    append_literal("str"sv);
    append_literal("sum"sv);
    append_literal("tuple"sv);
    append_literal("zip"sv);
    append_literal("print"sv);

    // OOP conveniences (decorators)
    append_literal("property"sv);
    append_literal("classmethod"sv);
    append_literal("staticmethod"sv);

    return builtins;
}

// Restricted open function that raises PermissionError when called
// This allows Python's internal code to check for 'open' without KeyError,
// but prevents user code from actually using it
static PyObject* restricted_open_func(PyObject* self, PyObject* args)
{
    PyErr_SetString(PyExc_PermissionError, "open() is not allowed in browser environment for security reasons");
    return nullptr;
}

Vector<String> dangerous_patterns()
{
    Vector<String> patterns;
    patterns.ensure_capacity(20);
    auto append_literal = [&](StringView literal) {
        patterns.unchecked_append(MUST(String::from_utf8(literal)));
    };
    append_literal("__import__"sv);
    append_literal("eval("sv);
    append_literal("exec("sv);
    append_literal("globals("sv);
    append_literal("locals("sv);
    append_literal("getattr("sv);
    append_literal("setattr("sv);
    append_literal("delattr("sv);
    append_literal("open("sv);
    append_literal("compile("sv);
    append_literal("input("sv);
    append_literal("subprocess"sv);
    append_literal("ctypes"sv);
    append_literal("os.system"sv);
    append_literal("sys.modules"sv);
    append_literal("importlib"sv);
    append_literal("__class__"sv);
    append_literal("__bases__"sv);
    append_literal("__subclasses__"sv);
    append_literal("builtins.__dict__"sv);
    return patterns;
}

Vector<String> default_allowed_modules()
{
    Vector<String> modules;
    modules.ensure_capacity(15);
    auto append_literal = [&](StringView literal) {
        modules.unchecked_append(MUST(String::from_utf8(literal)));
    };
    append_literal("math"sv);
    append_literal("random"sv);
    append_literal("statistics"sv);
    append_literal("datetime"sv);
    append_literal("json"sv);
    append_literal("collections"sv);
    append_literal("functools"sv);
    append_literal("itertools"sv);
    append_literal("operator"sv);
    append_literal("string"sv);
    append_literal("re"sv);
    append_literal("time"sv);
    append_literal("asyncio"sv);
    append_literal("decimal"sv);
    append_literal("pathlib"sv);
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

} // namespace

bool PythonSecurityModel::s_security_initialized = false;
HashMap<String, HashTable<String>> PythonSecurityModel::s_origin_allowed_modules;
HashTable<String> PythonSecurityModel::s_safe_domains;
HashMap<String, PythonSecurityModel::ResourceLimits> PythonSecurityModel::s_origin_resource_limits;

ErrorOr<void> PythonSecurityModel::initialize_security()
{
    if (s_security_initialized)
        return {};

    s_safe_domains.ensure_capacity(8);
    s_safe_domains.set(MUST(String::from_utf8("localhost"sv)));
    s_safe_domains.set(MUST(String::from_utf8("127.0.0.1"sv)));
    s_safe_domains.set(MUST(String::from_utf8("0.0.0.0"sv)));

    auto default_modules = default_allowed_modules();
    HashTable<String> module_table;
    for (auto const& module : default_modules)
        module_table.set(module);
    s_origin_allowed_modules.set(default_origin_key(), move(module_table));

    ResourceLimits default_limits;
    s_origin_resource_limits.set(default_origin_key(), default_limits);

    s_security_initialized = true;
    return {};
}

ErrorOr<bool> PythonSecurityModel::should_allow_script_execution(String const& script_content, URL::URL const& origin)
{
    (void)origin;
    TRY(initialize_security());

    if (!TRY(is_code_safe(script_content)))
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
    // Apply basic FS restrictions to neutralize obvious modules/APIs regardless of imports
    TRY(setup_restricted_filesystem_access(globals_ptr));

    auto limits = s_origin_resource_limits.get(normalize_origin(origin));
    if (!limits.has_value())
        limits = s_origin_resource_limits.get(default_origin_key());
    VERIFY(limits.has_value());
    TRY(set_resource_limits(globals_ptr, *limits));

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
        return Error::from_string_literal("Failed to create safe builtins dictionary");

    auto safe_names = default_safe_builtins();
    for (auto const& name : safe_names) {
        auto name_bytes = name.to_byte_string();
        PyObject* builtin = PyDict_GetItemString(builtins_module, name_bytes.characters());
        if (builtin) {
            Py_INCREF(builtin);
            PyDict_SetItemString(safe_builtins, name_bytes.characters(), builtin);
        }
    }
    
    // Create a restricted 'open' function that raises an error when called
    // This allows Python's internal code to check for 'open' without KeyError,
    // but prevents user code from actually using it
    static PyMethodDef restricted_open_method = {
        "open",
        (PyCFunction)restricted_open_func,
        METH_VARARGS,
        "Restricted: open() is not allowed"
    };
    
    PyObject* restricted_open = PyCFunction_New(&restricted_open_method, nullptr);
    if (restricted_open) {
        PyDict_SetItemString(safe_builtins, "open", restricted_open);
        Py_DECREF(restricted_open);
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
        auto module_view = module_name.bytes_as_string_view();
        auto allowed_view = allowed.bytes_as_string_view();
        if (module_view.starts_with(allowed_view) && module_view.length() > allowed_view.length() && module_view[allowed_view.length()] == '.')
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
        auto host_string = target_url.host().has_value() ? target_url.host()->serialize() : String {};
        bool allowed = false;
        for (auto const& domain : s_safe_domains) {
            if (host_matches(host_string.bytes_as_string_view(), domain.bytes_as_string_view())) {
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
    auto recursion_c_string = recursion_command.to_byte_string();
    if (PyRun_SimpleString(recursion_c_string.characters()) != 0)
        return Error::from_string_literal("Failed to set recursion limit");

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

ErrorOr<bool> PythonSecurityModel::check_against_csp(String const& code, URL::URL const& origin)
{
    (void)code;
    (void)origin;
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
    if (auto default_entry = s_origin_allowed_modules.get(default_origin_key()); default_entry.has_value()) {
        modules.ensure_capacity(default_entry->size());
        for (auto const& module : *default_entry)
            modules.append(module);
    }
    return modules;
}

HashMap<String, double> PythonSecurityModel::get_resource_usage(void* interpreter)
{
    (void)interpreter;
    HashMap<String, double> usage;
    usage.set(MUST(String::from_utf8("cpu_time_ms"sv)), 0.0);
    usage.set(MUST(String::from_utf8("memory_bytes"sv)), 0.0);
    usage.set(MUST(String::from_utf8("executions"sv)), 0.0);
    return usage;
}

// Helper methods
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
    (void)interpreter;
#if defined(AK_OS_UNIX)
    String command = MUST(String::formatted(
        "import resource; resource.setrlimit(resource.RLIMIT_AS, ({}, {}))", max_bytes, max_bytes));
    auto command_c_str = command.to_byte_string();
    if (PyRun_SimpleString(command_c_str.characters()) != 0)
        return Error::from_string_literal("Failed to set memory limit");
#else
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
    auto command_c_str = command.to_byte_string();
    if (PyRun_SimpleString(command_c_str.characters()) != 0)
        return Error::from_string_literal("Failed to configure CPU limiter");
#else
    (void)max_ms;
#endif
    return {};
}

} // namespace Web::HTML
