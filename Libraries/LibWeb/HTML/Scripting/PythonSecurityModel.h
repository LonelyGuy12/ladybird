/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Python.h>
#include <LibWeb/Forward.h>

namespace Web::HTML {

class PythonSecurityModel {
public:
    // Initialize the security model
    static bool initialize_security();
    
    // Check if a Python script should be allowed to run
    static bool should_allow_script_execution(const String& script_content, const URL& origin);
    
    // Set up sandboxed execution environment for Python
    static bool setup_sandboxed_environment(PyObject* globals, const URL& origin);
    
    // Restrict Python built-ins to safe operations only
    static bool restrict_builtins(PyObject* globals);
    
    // Check if a module import should be allowed
    static bool should_allow_module_import(const String& module_name, const URL& origin);
    
    // Set up restricted file system access
    static bool setup_restricted_filesystem_access();
    
    // Check if a network request should be allowed
    static bool should_allow_network_request(const URL& target_url, const URL& origin);
    
private:
    static bool s_security_initialized;
};

}