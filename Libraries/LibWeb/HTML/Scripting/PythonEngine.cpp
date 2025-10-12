/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <Python.h>
#include <LibWeb/HTML/Scripting/PythonEngine.h>
#include <LibWeb/Bindings/MainThreadVM.h>
#include <LibJS/Runtime/Realm.h>
#include <LibWeb/Bindings/PythonDOMBindings.h>
#include <LibWeb/HTML/Scripting/PythonSecurityModel.h>

namespace Web::HTML {

bool PythonEngine::s_initialized = false;

void PythonEngine::initialize()
{
    if (s_initialized) {
        return;
    }
    
    // Initialize Python interpreter
    Py_Initialize();
    
    // Initialize GIL
    PyEval_InitThreads();
    
    // Release GIL since we'll acquire it as needed
    PyThreadState* mainThreadState = PyEval_SaveThread();
    
    // Initialize Python DOM API
    Web::Bindings::PythonDOMAPI::initialize_module();
    
    // Initialize Python security model
    Web::HTML::PythonSecurityModel::initialize_security();
    
    s_initialized = true;
}

void PythonEngine::shutdown()
{
    if (!s_initialized) {
        return;
    }
    
    PyGILState_Ensure();  // Acquire GIL before finalizing
    Py_Finalize();
    s_initialized = false;
}

bool PythonEngine::is_initialized()
{
    return s_initialized;
}

void* PythonEngine::get_main_interpreter_state()
{
    if (!s_initialized) {
        return nullptr;
    }
    
    // Note: This is a simplified approach. In a real implementation,
    // we'd need more sophisticated interpreter state management
    return PyGILState_GetThisThreadState();
}

void* PythonEngine::create_subinterpreter()
{
    if (!s_initialized) {
        return nullptr;
    }
    
    PyGILState_STATE gstate = PyGILState_Ensure();
    
    // Create a new subinterpreter
    PyThreadState* subThreadState = Py_NewInterpreter();
    
    PyGILState_Release(gstate);
    
    return subThreadState;
}

void PythonEngine::destroy_subinterpreter(void* subinterpreter)
{
    if (!s_initialized || !subinterpreter) {
        return;
    }
    
    PyGILState_STATE gstate = PyGILState_Ensure();
    
    PyThreadState* subThreadState = static_cast<PyThreadState*>(subinterpreter);
    
    // Restore thread state to main interpreter before deleting
    Py_EndInterpreter(subThreadState);
    
    PyGILState_Release(gstate);
}

}