/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Forward.h>
#include <LibWeb/Forward.h>

namespace Web::HTML {

class PythonEngine {
public:
    static void initialize();
    static void shutdown();
    
    // Check if Python is initialized
    static bool is_initialized();
    
    // Get the main interpreter state
    static void* get_main_interpreter_state();
    
    // Create a new subinterpreter for isolation
    static void* create_subinterpreter();
    static void destroy_subinterpreter(void* subinterpreter);
    
private:
    static bool s_initialized;
};

}