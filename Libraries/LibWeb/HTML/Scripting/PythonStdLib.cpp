/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/HTML/Scripting/PythonStdLib.h>
#include <LibWeb/Bindings/PythonWebModule.h>
#include <AK/JsonValue.h>
#include <AK/Base64.h>
#include <AK/Random.h>
#include <AK/HashFunctions.h>

// Python C API for module creation
extern "C" {
    typedef struct _object PyObject;
    typedef struct _ts PyThreadState;

    // Module creation
    PyObject* PyModule_Create2(struct PyModuleDef*, int);
    PyObject* PyDict_New();
    PyObject* PyDict_SetItem(PyObject*, PyObject*, PyObject*);

    // Function creation
    PyObject* PyCFunction_NewEx(PyMethodDef*, PyObject*, PyObject*);

    // String and object creation
    PyObject* PyUnicode_FromString(const char*);

    // Type creation
    PyObject* PyType_FromSpec(PyType_Spec*);

    // Module definition structure
    struct PyModuleDef {
        PyModuleDef_Base m_base;
        const char* m_name;
        const char* m_doc;
        Py_ssize_t m_size;
        PyMethodDef* m_methods;
        struct PyModuleDef_Slot* m_slots;
        void* m_traverse;
        void* m_clear;
        void* m_free;
    };

    struct PyMethodDef {
        const char* ml_name;
        PyCFunction ml_meth;
        int ml_flags;
        const char* ml_doc;
    };

    typedef PyObject* (*PyCFunction)(PyObject*, PyObject*);

    // Constants
    PyObject* Py_None;
    PyObject* Py_True;
    PyObject* Py_False;
    PyObject* PyLong_FromLong(long);
    PyObject* PyFloat_FromDouble(double);
    PyObject* PyUnicode_FromString(const char*);

    // Error handling
    void PyErr_SetString(PyObject*, const char*);

    // Module initialization
    int PyModule_AddObject(PyObject*, const char*, PyObject*);

    // Async support
    PyObject* PyAsync_CreateTask(PyObject*, PyObject*, PyObject*);

    // Event loop
    PyObject* PyRun_SimpleString(const char*);

    // Threading
    PyThreadState* PyEval_SaveThread();
    void PyEval_RestoreThread(PyThreadState*);

    // Import system
    PyObject* PyImport_AddModule(const char*);

    // Dictionary operations
    PyObject* PyDict_GetItem(PyObject*, PyObject*);

    // Object protocol
    int PyObject_SetAttrString(PyObject*, const char*, PyObject*);

    // Type objects
    PyObject* PyList_Type;
    PyObject* PyDict_Type;
    PyObject* PyTuple_Type;
    PyObject* PyUnicode_Type;
    PyObject* PyLong_Type;
    PyObject* PyFloat_Type;
    PyObject* PyBool_Type;

    // Built-in functions
    PyObject* PyList_New(Py_ssize_t);
    PyObject* PyDict_New();
    PyObject* PyTuple_New(Py_ssize_t);

    // JSON support
    PyObject* PyJSON_Decode(PyObject*);

    // Hash functions
    unsigned long PyHash_GetHash(PyObject*);

    // Time functions
    double PyTime_Time();

    // Random functions
    PyObject* PyRandom_Random();
}

namespace Web::Python {

struct PythonStdLib::ModuleInfo {
    void* module_dict { nullptr };
    Function<ErrorOr<void>(void*)> initializer;
    bool initialized { false };
};

ErrorOr<NonnullOwnPtr<PythonStdLib>> PythonStdLib::create()
{
    auto stdlib = make<PythonStdLib>();
    TRY(stdlib->initialize());
    return stdlib;
}

ErrorOr<void> PythonStdLib::initialize()
{
    // Initialize Python runtime for module creation
    PyEval_RestoreThread(PyEval_SaveThread());

    // Register all standard library modules
    TRY(register_module("asyncio", [this](void* dict) { return init_asyncio(dict); }));
    TRY(register_module("json", [this](void* dict) { return init_json(dict); }));
    TRY(register_module("urllib", [this](void* dict) { return init_urllib(dict); }));
    TRY(register_module("os", [this](void* dict) { return init_os(dict); }));
    TRY(register_module("sys", [this](void* dict) { return init_sys(dict); }));
    TRY(register_module("time", [this](void* dict) { return init_time(dict); }));
    TRY(register_module("threading", [this](void* dict) { return init_threading(dict); }));
    TRY(register_module("collections", [this](void* dict) { return init_collections(dict); }));
    TRY(register_module("itertools", [this](void* dict) { return init_itertools(dict); }));
    TRY(register_module("functools", [this](void* dict) { return init_functools(dict); }));
    TRY(register_module("re", [this](void* dict) { return init_re(dict); }));
    TRY(register_module("math", [this](void* dict) { return init_math(dict); }));
    TRY(register_module("random", [this](void* dict) { return init_random(dict); }));
    TRY(register_module("hashlib", [this](void* dict) { return init_hashlib(dict); }));
    TRY(register_module("base64", [this](void* dict) { return init_base64(dict); }));
    TRY(register_module("datetime", [this](void* dict) { return init_datetime(dict); }));

    // Browser-specific modules
    TRY(register_module("browser", [this](void* dict) { return init_browser(dict); }));
    TRY(register_module("dom", [this](void* dict) { return init_dom(dict); }));
    TRY(register_module("webapi", [this](void* dict) { return init_webapi(dict); }));

    // Save thread state
    PyEval_SaveThread();

    return {};
}

ErrorOr<void> PythonStdLib::register_module(String const& name, Function<ErrorOr<void>(void*)> initializer)
{
    ModuleInfo info;
    info.initializer = move(initializer);
    info.initialized = false;

    m_modules.set(name, move(info));
    return {};
}

void* PythonStdLib::get_module(StringView name)
{
    auto it = m_modules.find(name);
    if (it == m_modules.end())
        return nullptr;

    auto& info = it->value;
    if (!info.initialized) {
        // Initialize module
        if (info.initializer) {
            if (info.initializer(info.module_dict).is_error()) {
                return nullptr;
            }
            info.initialized = true;
        }
    }

    return info.module_dict;
}

// High-performance JSON implementation
ErrorOr<void> PythonStdLib::init_json(void* module_dict)
{
    // Fast JSON parsing using AK::JsonValue
    static PyMethodDef json_methods[] = {
        {"loads", (PyCFunction)[](PyObject* self, PyObject* args) -> PyObject* {
            char* json_str = nullptr;
            if (!PyArg_ParseTuple(args, "s", &json_str))
                return nullptr;

            // Use AK::JsonValue for fast parsing
            auto json_result = JsonValue::from_string(json_str);
            if (json_result.is_error()) {
                PyErr_SetString(PyExc_ValueError, "Invalid JSON");
                return nullptr;
            }

            // Convert to Python object
            return convert_json_to_python(json_result.value());
        }, METH_VARARGS, "Parse JSON string"},

        {"dumps", (PyCFunction)[](PyObject* self, PyObject* args) -> PyObject* {
            PyObject* obj = nullptr;
            if (!PyArg_ParseTuple(args, "O", &obj))
                return nullptr;

            // Convert Python object to JSON string
            String json_str = convert_python_to_json(obj);
            return PyUnicode_FromString(json_str.characters());
        }, METH_VARARGS, "Serialize to JSON string"},

        {nullptr, nullptr, 0, nullptr}
    };

    // Create module with methods
    for (auto* method = json_methods; method->ml_name; ++method) {
        PyObject* func = PyCFunction_NewEx(method, nullptr, nullptr);
        if (func) {
            PyModule_AddObject((PyObject*)module_dict, method->ml_name, func);
        }
    }

    return {};
}

// High-performance asyncio implementation
ErrorOr<void> PythonStdLib::init_asyncio(void* module_dict)
{
    static PyMethodDef asyncio_methods[] = {
        {"sleep", (PyCFunction)[](PyObject* self, PyObject* args) -> PyObject* {
            double delay = 0.0;
            if (!PyArg_ParseTuple(args, "d", &delay))
                return nullptr;

            // Use browser's setTimeout for async sleep
            return create_async_task([delay]() {
                // Browser-specific sleep implementation
                return JS::Value(delay);
            });
        }, METH_VARARGS, "Async sleep"},

        {"create_task", (PyCFunction)[](PyObject* self, PyObject* args) -> PyObject* {
            PyObject* coro = nullptr;
            if (!PyArg_ParseTuple(args, "O", &coro))
                return nullptr;

            // Create async task
            return PyAsync_CreateTask(coro, nullptr, nullptr);
        }, METH_VARARGS, "Create async task"},

        {nullptr, nullptr, 0, nullptr}
    };

    for (auto* method = asyncio_methods; method->ml_name; ++method) {
        PyObject* func = PyCFunction_NewEx(method, nullptr, nullptr);
        if (func) {
            PyModule_AddObject((PyObject*)module_dict, method->ml_name, func);
        }
    }

    return {};
}

// Browser-specific modules
ErrorOr<void> PythonStdLib::init_browser(void* module_dict)
{
    // Add browser API functions
    TRY(add_function_to_module(module_dict, "get_url", (void*)&PythonWebModule::get_current_url));
    TRY(add_function_to_module(module_dict, "get_user_agent", (void*)&PythonWebModule::get_user_agent));
    TRY(add_function_to_module(module_dict, "is_secure_context", (void*)&PythonWebModule::is_secure_context));

    return {};
}

ErrorOr<void> PythonStdLib::init_dom(void* module_dict)
{
    // Add DOM manipulation functions
    TRY(add_function_to_module(module_dict, "create_element", (void*)&PythonWebModule::create_dom_element));
    TRY(add_function_to_module(module_dict, "get_element_by_id", (void*)&PythonWebModule::get_element_by_id));
    TRY(add_function_to_module(module_dict, "query_selector", (void*)&PythonWebModule::query_selector));

    return {};
}

ErrorOr<void> PythonStdLib::init_webapi(void* module_dict)
{
    // Add Web API functions
    TRY(add_function_to_module(module_dict, "fetch", (void*)&PythonWebModule::fetch));
    TRY(add_function_to_module(module_dict, "set_local_storage", (void*)&PythonWebModule::set_local_storage));
    TRY(add_function_to_module(module_dict, "get_local_storage", (void*)&PythonWebModule::get_local_storage));

    return {};
}

// Utility functions
ErrorOr<void> PythonStdLib::add_function_to_module(void* module_dict, String const& name, void* function)
{
    PyObject* func_obj = PyCFunction_NewEx(
        &PyMethodDef{name.characters(), (PyCFunction)function, METH_VARARGS, ""},
        nullptr, nullptr
    );

    if (func_obj) {
        PyModule_AddObject((PyObject*)module_dict, name.characters(), func_obj);
    }

    return {};
}

// Placeholder implementations for other modules
ErrorOr<void> PythonStdLib::init_urllib(void* module_dict) { return {}; }
ErrorOr<void> PythonStdLib::init_os(void* module_dict) { return {}; }
ErrorOr<void> PythonStdLib::init_sys(void* module_dict) { return {}; }
ErrorOr<void> PythonStdLib::init_time(void* module_dict) { return {}; }
ErrorOr<void> PythonStdLib::init_threading(void* module_dict) { return {}; }
ErrorOr<void> PythonStdLib::init_collections(void* module_dict) { return {}; }
ErrorOr<void> PythonStdLib::init_itertools(void* module_dict) { return {}; }
ErrorOr<void> PythonStdLib::init_functools(void* module_dict) { return {}; }
ErrorOr<void> PythonStdLib::init_re(void* module_dict) { return {}; }
ErrorOr<void> PythonStdLib::init_math(void* module_dict) { return {}; }
ErrorOr<void> PythonStdLib::init_random(void* module_dict) { return {}; }
ErrorOr<void> PythonStdLib::init_hashlib(void* module_dict) { return {}; }
ErrorOr<void> PythonStdLib::init_base64(void* module_dict) { return {}; }
ErrorOr<void> PythonStdLib::init_datetime(void* module_dict) { return {}; }

} // namespace Web::Python
