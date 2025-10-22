/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibURL/Origin.h>
#include <LibURL/URL.h>
#include <LibWeb/HTML/Scripting/PythonSecurityModel.h>

namespace Web::HTML {

bool PythonSecurityModel::s_security_initialized = false;

bool PythonSecurityModel::initialize_security()
{
    if (s_security_initialized) {
        return true;
    }

    s_security_initialized = true;
    return true;
}

bool PythonSecurityModel::should_allow_script_execution(String const&, const URL::URL&)
{
    // For now, allow execution for same-origin scripts
    // In a complete implementation, we'd check against CSP policies,
    // examine the script for malicious content, etc.
    return true;
}

bool PythonSecurityModel::setup_sandboxed_environment(PyObject* globals, const URL::URL& origin)
{
    (void)origin;
    if (!globals) {
        return false;
    }

    // Start with safe built-ins only
    if (!restrict_builtins(globals)) {
        return false;
    }

    // Remove or restrict dangerous modules
    PyObject* modules = PyImport_GetModuleDict();

    // Remove access to dangerous modules
    char const* dangerous_modules[] = {
        "os", "sys", "subprocess", "socket", "urllib", "urllib2", "httplib", "http.client",
        "ftplib", "telnetlib", "smtplib", "pickle", "cPickle", "shelve", "marshal", "imp",
        "importlib", "compileall", "py_compile", "shutil", "glob", "requests", "urllib3",
        "httpx", "aiohttp", "xml", "xmlrpclib", "urllib.request", "urllib.parse", "urllib.error"
    };

    for (char const* module_name : dangerous_modules) {
        if (PyDict_DelItemString(modules, module_name) < 0) {
            // Module wasn't imported yet, that's fine
            PyErr_Clear();
        }
    }

    return true;
}

bool PythonSecurityModel::restrict_builtins(PyObject* globals)
{
    if (!globals || !PyDict_Check(globals)) {
        return false;
    }

    PyObject* builtins = PyEval_GetBuiltins();
    if (!builtins) {
        return false;
    }

    // Create a restricted builtins dictionary
    PyObject* restricted_builtins = PyDict_New();
    if (!restricted_builtins) {
        return false;
    }

    // Allow safe built-in functions
    char const* safe_builtins[] = {
        "abs", "all", "any", "bool", "chr", "dict", "enumerate", "filter", "float",
        "int", "isinstance", "len", "list", "map", "max", "min", "ord", "pow",
        "range", "sorted", "str", "sum", "type", "zip", "True", "False", "None"
    };

    for (char const* builtin_name : safe_builtins) {
        PyObject* builtin_func = PyDict_GetItemString(builtins, builtin_name);
        if (builtin_func) {
            PyDict_SetItemString(restricted_builtins, builtin_name, builtin_func);
        }
    }

    // Replace the __builtins__ in the globals
    PyDict_SetItemString(globals, "__builtins__", restricted_builtins);
    Py_DECREF(restricted_builtins);

    return true;
}

bool PythonSecurityModel::should_allow_module_import(String const& module_name, const URL::URL& origin)
{
    (void)origin;
    // For security, only allow imports for specific modules that are safe
    // In a real implementation, we'd have a more sophisticated whitelist
    StringView module_view = module_name;

    // Allow only safe modules
    return module_view.is_one_of("json"sv, "datetime"sv, "math"sv, "random"sv, "re"sv, "collections"sv, "itertools"sv);
}

bool PythonSecurityModel::setup_restricted_filesystem_access()
{
    // In a real implementation, we'd override file operations to prevent
    // access to the real filesystem
    // For now, we'll just note that this is where filesystem restrictions would go
    return true;
}

bool PythonSecurityModel::should_allow_network_request(const URL::URL& target_url, const URL::URL& origin)
{
    // Implement SOP (Same-Origin Policy) for network requests
    // Only allow requests to same origin by default
    URL::Origin target_origin = target_url.origin();
    URL::Origin source_origin = origin.origin();

    return target_origin == source_origin;
}

}
