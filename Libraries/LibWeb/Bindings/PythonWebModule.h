/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Python.h>
#include <LibWeb/Forward.h>

namespace Web::Bindings {

class PythonWebModule {
public:
    static bool initialize_web_module();
    static PyObject* get_web_module();
    
private:
    static PyObject* s_web_module;
    static bool s_initialized;
};

}