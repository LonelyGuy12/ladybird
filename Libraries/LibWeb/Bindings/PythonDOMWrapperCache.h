/*
 * Copyright (c) 2025, Ladybird contributors
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/HashMap.h>
#include <Python.h>

namespace Web::Bindings {

// Per-document cache for Python wrappers
class PythonDOMWrapperCache {
public:
    ~PythonDOMWrapperCache()
    {
        for (auto const& it : m_wrapper_cache) {
            Py_DECREF(it.value);
        }
    }

    PyObject* get_wrapper(void* cpp_object)
    {
        if (auto it = m_wrapper_cache.find(cpp_object); it != m_wrapper_cache.end()) {
            Py_INCREF(it->value);
            return it->value;
        }
        return nullptr;
    }

    void set_wrapper(void* cpp_object, PyObject* wrapper)
    {
        m_wrapper_cache.set(cpp_object, wrapper);
        Py_INCREF(wrapper);
    }

private:
    HashMap<void*, PyObject*> m_wrapper_cache;
};

}
