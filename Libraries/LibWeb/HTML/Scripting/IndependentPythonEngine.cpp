/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/ByteString.h>
#include <AK/HashMap.h>
#include <AK/NonnullOwnPtr.h>
#include <AK/ScopeGuard.h>
#include <AK/String.h>
#include <AK/StringView.h>
#include <AK/Try.h>
#include <AK/Vector.h>
#include <LibJS/Runtime/Array.h>
#include <LibJS/Runtime/PrimitiveString.h>
#include <LibJS/Runtime/VM.h>
#include <LibWeb/Bindings/MainThreadVM.h>
#include <LibWeb/HTML/Scripting/IndependentPythonEngine.h>
#include <LibWeb/HTML/Scripting/PythonError.h>
#include <LibWeb/HTML/Scripting/PythonPerformanceMetrics.h>

#include <Python.h>
#include <cstring>

namespace Web::HTML {

namespace {

inline ByteString sanitized_filename(StringView filename)
{
    if (filename.is_empty())
        return "<string>"sv;
    return filename.to_byte_string();
}

JS::Value python_object_to_js(JS::Realm& realm, PyObject* object)
{
    if (!object)
        return JS::js_undefined();

    if (object == Py_None)
        return JS::js_undefined();

    if (PyBool_Check(object))
        return JS::Value(object == Py_True);

    if (PyLong_Check(object)) {
        auto value = PyLong_AsLongLong(object);
        if (PyErr_Occurred()) {
            PyErr_Clear();
            return JS::js_undefined();
        }
        return JS::Value(static_cast<double>(value));
    }

    if (PyFloat_Check(object)) {
        auto value = PyFloat_AsDouble(object);
        if (PyErr_Occurred()) {
            PyErr_Clear();
            return JS::js_undefined();
        }
        return JS::Value(value);
    }

    if (PyUnicode_Check(object)) {
        auto const* utf8 = PyUnicode_AsUTF8(object);
        if (!utf8) {
            PyErr_Clear();
            return JS::js_undefined();
        }
        auto const* utf8_begin = utf8;
        auto string_view = StringView { utf8_begin, strlen(utf8_begin) };
        auto string = MUST(String::from_utf8(string_view));
        return JS::PrimitiveString::create(realm.vm(), move(string));
    }

    if (PyList_Check(object)) {
        auto array = MUST(JS::Array::create(realm, 0)).ptr();
        auto size = PyList_Size(object);
        for (Py_ssize_t index = 0; index < size; ++index) {
            PyObject* element = PyList_GetItem(object, index); // Borrowed reference
            if (!element)
                continue;
            array->indexed_properties().append(python_object_to_js(realm, element));
        }
        return JS::Value(array);
    }

    if (PyTuple_Check(object)) {
        auto array = MUST(JS::Array::create(realm, 0)).ptr();
        auto size = PyTuple_Size(object);
        for (Py_ssize_t index = 0; index < size; ++index) {
            PyObject* element = PyTuple_GetItem(object, index); // Borrowed reference
            if (!element)
                continue;
            array->indexed_properties().append(python_object_to_js(realm, element));
        }
        return JS::Value(array);
    }

    return JS::js_undefined();
}

} // namespace

struct IndependentPythonEngine::Impl {
    ~Impl()
    {
        if (globals) {
            Py_DECREF(globals);
            globals = nullptr;
        }
        if (locals) {
            Py_DECREF(locals);
            locals = nullptr;
        }
        for (auto& entry : compiled_cache) {
            Py_DECREF(entry.value);
        }
        compiled_cache.clear();
    }

    PyObject* globals { nullptr };
    PyObject* locals { nullptr };
    HashMap<ByteString, PyObject*> compiled_cache;
    PythonPerformanceMetrics::ExecutionStats stats;
};

ErrorOr<NonnullOwnPtr<IndependentPythonEngine>> IndependentPythonEngine::create()
{
    auto engine = adopt_own(*new IndependentPythonEngine);
    TRY(engine->initialize());
    return engine;
}

IndependentPythonEngine::~IndependentPythonEngine()
{
    if (!m_impl)
        return;

    PyGILState_STATE gstate = PyGILState_Ensure();
    m_impl = nullptr;
    PyGILState_Release(gstate);
}

ErrorOr<void> IndependentPythonEngine::initialize()
{
    if (m_initialized)
        return {};

    if (!Py_IsInitialized())
        Py_Initialize();

    PyGILState_STATE gstate = PyGILState_Ensure();

    m_impl = make<Impl>();

    m_impl->globals = PyDict_New();
    if (!m_impl->globals) {
        PyGILState_Release(gstate);
        return Error::from_string_literal("Failed to allocate Python globals dictionary");
    }

    m_impl->locals = PyDict_New();
    if (!m_impl->locals) {
        PyGILState_Release(gstate);
        return Error::from_string_literal("Failed to allocate Python locals dictionary");
    }

    PyObject* builtins = PyEval_GetBuiltins();
    if (builtins) {
        Py_INCREF(builtins);
        PyDict_SetItemString(m_impl->globals, "__builtins__", builtins);
        Py_DECREF(builtins);
    }

    auto const* version_cstr = Py_GetVersion();
    auto version_view = StringView { version_cstr, strlen(version_cstr) };
    if (auto version_result = String::from_utf8(version_view); !version_result.is_error())
        m_version = version_result.release_value();

    m_initialized = true;

    PyGILState_Release(gstate);

    PythonPerformanceMetrics::end_timing();
    m_impl->stats = PythonPerformanceMetrics::get_current_stats();

    return {};
}

ErrorOr<JS::Value> IndependentPythonEngine::run(StringView source, StringView filename)
{
    if (!m_initialized)
        TRY(initialize());

    PythonPerformanceMetrics::start_timing();
    PythonPerformanceMetrics::record_function_call();

    PyGILState_STATE gstate = PyGILState_Ensure();

    auto guard = ScopeGuard([&]() {
        PyGILState_Release(gstate);
        m_impl->stats = PythonPerformanceMetrics::end_timing();
    });

    auto cache_key = sanitized_filename(filename);

    PyObject* compiled_code = nullptr;
    if (auto cached = m_impl->compiled_cache.get(cache_key); cached.has_value()) {
        compiled_code = cached.value();
        Py_INCREF(compiled_code);
    } else {
        auto source_bytes = source.to_byte_string();
        compiled_code = Py_CompileString(source_bytes.characters(), cache_key.characters(), Py_file_input);
        if (!compiled_code)
            return PythonError::from_python_exception();

        if (auto insert_result = m_impl->compiled_cache.try_set(cache_key, compiled_code); insert_result.is_error()) {
            Py_DECREF(compiled_code);
            return insert_result.release_error();
        }
        Py_INCREF(compiled_code);
    }

    auto* result = PyEval_EvalCode(compiled_code, m_impl->globals, m_impl->locals);
    Py_DECREF(compiled_code);

    if (!result)
        return PythonError::from_python_exception();

    auto& vm = Bindings::main_thread_vm();
    auto* realm = vm.current_realm();
    if (!realm)
        return Error::from_string_literal("No active JavaScript realm");
    auto js_result = python_object_to_js(*realm, result);
    Py_DECREF(result);

    return js_result;
}

ErrorOr<JS::Value> IndependentPythonEngine::run_module(StringView module_name)
{
    if (!m_initialized)
        TRY(initialize());

    PythonPerformanceMetrics::start_timing();
    PythonPerformanceMetrics::record_function_call();

    PyGILState_STATE gstate = PyGILState_Ensure();

    auto guard = ScopeGuard([&]() {
        PyGILState_Release(gstate);
        m_impl->stats = PythonPerformanceMetrics::end_timing();
    });

    auto module_name_bytes = module_name.to_byte_string();
    auto* module = PyImport_ImportModule(module_name_bytes.characters());
    if (!module)
        return PythonError::from_python_exception();

    auto& vm = Bindings::main_thread_vm();
    auto* realm = vm.current_realm();
    if (!realm)
        return Error::from_string_literal("No active JavaScript realm");
    auto js_result = python_object_to_js(*realm, module);
    Py_DECREF(module);

    return js_result;
}

JS::Value IndependentPythonEngine::convert_python_to_js(void* py_obj)
{
    auto& vm = Bindings::main_thread_vm();
    auto* realm = vm.current_realm();
    if (!realm)
        return JS::js_undefined();
    return python_object_to_js(*realm, static_cast<PyObject*>(py_obj));
}

PythonPerformanceMetrics::ExecutionStats IndependentPythonEngine::get_performance_stats() const
{
    if (!m_impl)
        return {};
    return m_impl->stats;
}

} // namespace Web::HTML
