/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Debug.h>
#include <LibCore/ElapsedTimer.h>
#include <LibWeb/Bindings/ExceptionOrUtils.h>
#include <LibWeb/Bindings/PythonDOMBindings.h>
#include <LibWeb/HTML/Scripting/Environments.h>
#include <LibWeb/HTML/Scripting/ExceptionReporter.h>
#include <LibWeb/HTML/Scripting/PythonEngine.h>
#include <LibWeb/HTML/Scripting/PythonScript.h>
#include <LibWeb/HTML/Scripting/PythonSecurityModel.h>
#include <LibWeb/HTML/WindowOrWorkerGlobalScope.h>
#include <LibWeb/HTML/Window.h>
#include <LibWeb/WebIDL/DOMException.h>
#include <AK/TypeCasts.h>
#include <Python.h>
#include <cstring>

namespace Web::HTML {

GC_DEFINE_ALLOCATOR(PythonScript);

// https://html.spec.whatwg.org/multipage/webappapis.html#creating-a-python-script
GC::Ref<PythonScript> PythonScript::create(ByteString filename, StringView source, JS::Realm& realm, URL::URL base_url, MutedErrors muted_errors)
{
    auto& vm = realm.vm();

    // 1. If muted errors is true, then set baseURL to about:blank.
    if (muted_errors == MutedErrors::Yes)
        base_url = URL::about_blank();

    // 2. If scripting is disabled for realm, then set source to the empty string.
    if (is_scripting_disabled(realm))
        source = ""sv;

    // 3. Let script be a new python script that this algorithm will subsequently initialize.
    // 4. Set script's realm to realm.
    // 5. Set script's base URL to baseURL.
    auto script = vm.heap().allocate<PythonScript>(move(base_url), move(filename), realm);

    // 6. Set script's muted errors to muted errors.
    script->m_muted_errors = muted_errors;

    // 7. Set script's parse error and error to rethrow to null.
    script->set_parse_error(JS::js_null());
    script->set_error_to_rethrow(JS::js_null());

    // 8. Parse Python source code
    auto parse_timer = Core::ElapsedTimer::start_new();

    dbgln("🐍 PythonScript: Compiling Python code ({} bytes)", source.length());
    
    // Convert StringView to Python-compatible string
    auto source_bytes = source.to_byte_string();
    PyObject* compiled_code = Py_CompileString(source_bytes.characters(), filename.characters(), Py_file_input);

    if (!compiled_code) {
        // Handle compilation error
        dbgln("🐍 PythonScript: ❌ Failed to compile Python code");
        PyErr_Print();

        // Create a simple error
        script->set_parse_error(JS::SyntaxError::create(realm, "Python compilation failed"sv));
        script->set_error_to_rethrow(script->parse_error());
        return script;
    }
    
    dbgln("🐍 PythonScript: ✅ Compilation successful");

    // 9. Set script's record to compiled code.
    script->m_script_record = compiled_code;

    dbgln_if(HTML_SCRIPT_DEBUG, "PythonScript: Parsed {} in {}ms", script->filename(), parse_timer.elapsed_milliseconds());

    // 10. Return script.
    return script;
}

// https://html.spec.whatwg.org/multipage/webappapis.html#run-a-python-script
JS::Completion PythonScript::run(RethrowErrors rethrow_errors, GC::Ptr<JS::Environment> lexical_environment_override)
{
    (void)lexical_environment_override; // TODO: Use when implementing lexical environment support
    {
        // 1. Let realm be the realm of script.
        auto& realm = this->realm();

        // 2. Check if we can run script with realm. If this returns "do not run", then return NormalCompletion(empty).
        if (can_run_script(realm) == RunScriptDecision::DoNotRun)
            return JS::normal_completion(JS::js_undefined());

        // 3. Prepare to run script given realm.
        prepare_to_run_script(realm);

        // 4. Let evaluationStatus be null.
        JS::Completion evaluation_status = JS::normal_completion(JS::js_undefined());

        // 5. If script's error to rethrow is not null, then set evaluationStatus to ThrowCompletion(script's error to rethrow).
        if (!error_to_rethrow().is_null()) {
            evaluation_status = JS::throw_completion(error_to_rethrow());
        }
        // 6. Otherwise, execute Python script
        else {
        [[maybe_unused]] auto timer = Core::ElapsedTimer::start_new();
        
        dbgln("🐍 PythonScript::run() - Starting execution");

        // Execute the Python code
        if (m_script_record) {
            dbgln("🐍 PythonScript::run() - Has compiled code, executing...");
            // Create a new Python thread state to execute the script
            PyGILState_STATE gstate = PyGILState_Ensure();
            
            if (!m_execution_context) {
                m_execution_context = PyDict_New();
                // Provide minimal module-like globals
                PyObject* builtins = PyEval_GetBuiltins();
                if (builtins)
                    PyDict_SetItemString(m_execution_context, "__builtins__", builtins);
                PyDict_SetItemString(m_execution_context, "__name__", PyUnicode_FromString("__main__"));
                Py_INCREF(Py_None);
                PyDict_SetItemString(m_execution_context, "__package__", Py_None);
                Py_INCREF(Py_None);
                PyDict_SetItemString(m_execution_context, "__doc__", Py_None);
                Py_INCREF(Py_None);
                PyDict_SetItemString(m_execution_context, "__spec__", Py_None);
            }

            if (m_execution_context) {
                // Set up security restrictions for this script execution
                URL::URL origin = this->base_url().value_or(URL::URL {});
                auto security_result = PythonSecurityModel::setup_sandboxed_environment(m_execution_context, origin);
                if (security_result.is_error()) {
                    evaluation_status = JS::throw_completion(JS::Error::create(realm, "Failed to set up secure execution environment"sv));
                    PyGILState_Release(gstate);
                    return evaluation_status;
                }
                
                // Import and add the web module with Python-friendly APIs
                if (Bindings::PythonDOMAPI::initialize_module()) {
                    PyObject* web_module = Bindings::PythonDOMAPI::get_module();
                    if (web_module) {
                        PyDict_SetItemString(m_execution_context, "web", web_module);

                        // Expose Python wrapper types for convenience
                        PyObject* doc_class = PyObject_GetAttrString(web_module, "Document");
                        if (doc_class) {
                            PyDict_SetItemString(m_execution_context, "Document", doc_class);
                            Py_DECREF(doc_class);
                        }
                        PyObject* elem_class = PyObject_GetAttrString(web_module, "Element");
                        if (elem_class) {
                            PyDict_SetItemString(m_execution_context, "Element", elem_class);
                            Py_DECREF(elem_class);
                        }
                        PyObject* win_class = PyObject_GetAttrString(web_module, "Window");
                        if (win_class) {
                            PyDict_SetItemString(m_execution_context, "Window", win_class);
                            Py_DECREF(win_class);
                        }

                        // Expose current window and document instances to Python (no JS bridge)
                        auto& window_object = relevant_global_object(realm.global_object());
                        auto& win = verify_cast<Window>(window_object);
                        PyObject* py_window = Bindings::PythonWindow::create_from_cpp_window(win);
                        if (py_window) {
                            PyDict_SetItemString(m_execution_context, "window", py_window);
                            Py_DECREF(py_window);
                        }
                        auto doc_ref = win.document();
                        if (doc_ref) {
                            auto* cpp_doc = const_cast<DOM::Document*>(&*doc_ref);
                            PyObject* py_document = Bindings::PythonDocument::create_from_cpp_document(*cpp_doc);
                            if (py_document) {
                                PyDict_SetItemString(m_execution_context, "document", py_document);
                                Py_DECREF(py_document);
                            }
                        }
                    }
                }
                
                // Force UTF-8 encoding for stdout/stderr to handle emojis and unicode
                PyObject* sys_module = PyImport_ImportModule("sys");
                if (sys_module) {
                    // Flush stdout before execution
                    PyObject* stdout_obj = PyObject_GetAttrString(sys_module, "stdout");
                    if (stdout_obj && stdout_obj != Py_None) {
                        PyObject* flush_result = PyObject_CallMethod(stdout_obj, "flush", nullptr);
                        Py_XDECREF(flush_result);
                        Py_DECREF(stdout_obj);
                    }
                    Py_DECREF(sys_module);
                }
                
                dbgln("🐍 PythonScript::run() - Calling PyEval_EvalCode...");
                PyObject* result = PyEval_EvalCode(m_script_record, m_execution_context, m_execution_context);
                dbgln("🐍 PythonScript::run() - PyEval_EvalCode returned");
                
                // Flush stdout/stderr after execution to ensure output appears immediately
                sys_module = PyImport_ImportModule("sys");
                if (sys_module) {
                    PyObject* stdout_obj = PyObject_GetAttrString(sys_module, "stdout");
                    if (stdout_obj && stdout_obj != Py_None) {
                        PyObject* flush_result = PyObject_CallMethod(stdout_obj, "flush", nullptr);
                        Py_XDECREF(flush_result);
                        Py_DECREF(stdout_obj);
                    }
                    PyObject* stderr_obj = PyObject_GetAttrString(sys_module, "stderr");
                    if (stderr_obj && stderr_obj != Py_None) {
                        PyObject* flush_result = PyObject_CallMethod(stderr_obj, "flush", nullptr);
                        Py_XDECREF(flush_result);
                        Py_DECREF(stderr_obj);
                    }
                    Py_DECREF(sys_module);
                }
                
                if (!result) {
                    dbgln("🐍 PythonScript::run() - ❌ Execution failed with Python error");
                    // Python error occurred
                    PyObject* error_type = nullptr;
                    PyObject* error_value = nullptr;
                    PyObject* error_traceback = nullptr;
                    
                    PyErr_Fetch(&error_type, &error_value, &error_traceback);
                    PyErr_NormalizeException(&error_type, &error_value, &error_traceback);
                    
                    if (error_value) {
                        PyObject* error_str = PyObject_Str(error_value);
                        if (error_str) {
                            const char* error_cstr = PyUnicode_AsUTF8(error_str);
                            if (error_cstr) {
                                auto error_message = MUST(String::from_utf8(StringView { error_cstr, strlen(error_cstr) }));
                                evaluation_status = JS::throw_completion(JS::Error::create(realm, error_message));
                            }
                            Py_DECREF(error_str);
                        }
                        Py_DECREF(error_value);
                    }
                    if (error_type)
                        Py_DECREF(error_type);
                    if (error_traceback)
                        Py_DECREF(error_traceback);
                } else {
                    // Execution was successful
                    dbgln("🐍 PythonScript::run() - ✅ Execution successful!");
                    if (result != Py_None) {
                        // Currently unused; mark success
                    }
                    Py_DECREF(result);
                    evaluation_status = JS::normal_completion(JS::js_undefined());
                }

                PyGILState_Release(gstate);
            }
        }
        }

        // 7. If evaluationStatus is an abrupt completion, then:
        if (evaluation_status.is_abrupt()) {
            // 1. If rethrow errors is true and script's muted errors is false, then:
            if (rethrow_errors == RethrowErrors::Yes && m_muted_errors == MutedErrors::No) {
                // 1. Clean up after running script with realm.
                clean_up_after_running_script(realm);

                // 2. Rethrow evaluationStatus.[[Value]].
                return JS::throw_completion(evaluation_status.value());
            }

            // 2. If rethrow errors is true and script's muted errors is true, then:
            if (rethrow_errors == RethrowErrors::Yes && m_muted_errors == MutedErrors::Yes) {
                // 1. Clean up after running script with realm.
                clean_up_after_running_script(realm);

                // 2. Throw a "NetworkError" DOMException.
                return throw_completion(WebIDL::NetworkError::create(realm, "Script error."_utf16));
            }

            // 3. Otherwise, rethrow errors is false. Perform the following steps:
            VERIFY(rethrow_errors == RethrowErrors::No);

            // 1. Report an exception given by evaluationStatus.[[Value]] for realms's global object.
            auto& window_or_worker = as<WindowOrWorkerGlobalScopeMixin>(realm.global_object());
            window_or_worker.report_an_exception(evaluation_status.value());

            // 2. Clean up after running script with realm.
            clean_up_after_running_script(realm);

            // 3. Return evaluationStatus.
            return evaluation_status;
        }

        // 8. Clean up after running script with realm.
        clean_up_after_running_script(realm);

        // 9. If evaluationStatus is a normal completion, then return evaluationStatus.
        if (!evaluation_status.is_abrupt())
            return evaluation_status;

        // FIXME: 10. If we've reached this point, evaluationStatus was left as null because the script was aborted prematurely during evaluation.
        //            Return ThrowCompletion(a new "QuotaExceededError" DOMException).
        return JS::throw_completion(WebIDL::QuotaExceededError::create(realm, "Script execution was aborted"_utf16));
    }
}

PythonScript::PythonScript(URL::URL base_url, ByteString filename, JS::Realm& realm)
    : Script(move(base_url), move(filename), realm)
{
}

PythonScript::~PythonScript()
{
    if (m_script_record) {
        // Acquire GIL before destroying Python objects
        PyGILState_STATE gstate = PyGILState_Ensure();
        Py_DECREF(m_script_record);
        PyGILState_Release(gstate);
        m_script_record = nullptr;
    }
    if (m_execution_context) {
        PyGILState_STATE gstate = PyGILState_Ensure();
        Py_DECREF(m_execution_context);
        PyGILState_Release(gstate);
        m_execution_context = nullptr;
    }
}

void PythonScript::visit_edges(Cell::Visitor& visitor)
{
    Base::visit_edges(visitor);
    // Note: We don't visit m_script_record or m_execution_context since it's a Python object, not a GC object
}

}
