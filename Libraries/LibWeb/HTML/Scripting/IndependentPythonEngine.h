/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Error.h>
#include <AK/Noncopyable.h>
#include <AK/StringView.h>
#include <AK/String.h>
#include <LibWeb/Forward.h>
#include <LibJS/Forward.h>

namespace Web::HTML {

// Independent Python Engine that doesn't depend on JavaScript
class IndependentPythonEngine {
    AK_MAKE_NONCOPYABLE(IndependentPythonEngine);
    AK_MAKE_NONMOVABLE(IndependentPythonEngine);

public:
    static ErrorOr<NonnullOwnPtr<IndependentPythonEngine>> create();
    ~IndependentPythonEngine();

    // Initialize the Python interpreter
    ErrorOr<void> initialize();

    // Execute Python code in the global scope
    ErrorOr<JS::Value> run(StringView source, StringView filename = "<string>"sv);

    // Execute a Python module
    ErrorOr<JS::Value> run_module(StringView module_name);

    // Get the Python version
    StringView version() const { return m_version; }

    // Check if the engine is initialized
    bool is_initialized() const { return m_initialized; }

    // Get performance statistics
    // TODO: Implement performance metrics
    // PythonPerformanceMetrics::ExecutionStats get_performance_stats() const;

private:
    IndependentPythonEngine() = default;

    struct Impl;
    NonnullOwnPtr<Impl> m_impl;
    bool m_initialized { false };
    String m_version;

    // Convert Python object to JavaScript value
    JS::Value convert_python_to_js(void* py_obj);
};

} // namespace Web::HTML
