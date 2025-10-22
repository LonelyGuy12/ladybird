/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibURL/Origin.h>
#include <LibURL/URL.h>
#include <LibWeb/Forward.h>
#include <Python.h>

namespace Web::HTML {

class PythonSecurityModel {
public:
    // Initialize the security model
    static bool initialize_security();

    // Check if a Python script should be allowed to run
    static bool should_allow_script_execution(String const& script_content, URL::URL const& origin);

    // Set up sandboxed execution environment for Python
    static bool setup_sandboxed_environment(PyObject* globals, URL::URL const& origin);

    // Restrict Python built-ins to safe operations only
    static bool restrict_builtins(PyObject* globals);

    // Check if a module import should be allowed
    static bool should_allow_module_import(String const& module_name, URL::URL const& origin);

    // Set up restricted file system access
    static bool setup_restricted_filesystem_access();

    // Check if a network request should be allowed
    static bool should_allow_network_request(URL::URL const& target_url, URL::URL const& origin);

private:
    static bool s_security_initialized;
};

}
