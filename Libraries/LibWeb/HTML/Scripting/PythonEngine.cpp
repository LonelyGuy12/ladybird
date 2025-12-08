/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/ByteString.h>
#include <AK/Debug.h>
#include <LibJS/Runtime/Realm.h>
#include <LibWeb/Bindings/MainThreadVM.h>
#include <LibWeb/Bindings/PythonDOMBindings.h>
#include <LibWeb/HTML/Scripting/PythonDebugHelpers.h>
#include <LibWeb/HTML/Scripting/PythonEngine.h>
#include <LibWeb/HTML/Scripting/PythonSecurityModel.h>
#include <Python.h>

#ifdef __APPLE__
#include <limits.h>
#include <mach-o/dyld.h>
#include <sys/stat.h>
#include <wchar.h>
#endif

namespace Web::HTML {

#ifdef __APPLE__
// Helper to get app bundle path on macOS
static Optional<ByteString> get_bundled_python_home() {
  char exe_path[PATH_MAX];
  uint32_t size = sizeof(exe_path);
  if (_NSGetExecutablePath(exe_path, &size) == 0) {
    // exe_path is like: /path/to/Ladybird.app/Contents/MacOS/Ladybird
    ByteString path_str(exe_path);
    if (path_str.contains(".app/Contents/MacOS"sv)) {
      // Extract bundle path
      auto app_index = path_str.find(".app/Contents/MacOS"sv);
      if (app_index.has_value()) {
        auto bundle_path =
            path_str.substring(0, app_index.value() + 4); // Include ".app"
        auto python_home = ByteString::formatted(
            "{}/Contents/Resources/bundled_python/Versions/3.14", bundle_path);

        // Check if it exists
        struct stat buffer;
        if (stat(python_home.characters(), &buffer) == 0) {
          return python_home;
        }
      }
    }
  }
  return {};
}
#endif

bool PythonEngine::s_initialized = false;

void PythonEngine::initialize() {
  dbgln("🐍 PythonEngine::initialize() called");

  if (s_initialized) {
    dbgln("🐍 PythonEngine already initialized, skipping");
    return;
  }

  dbgln("🐍 Initializing Python interpreter...");

#ifdef __APPLE__
  // Set PYTHONHOME to bundled Python if available
  auto bundled_python = get_bundled_python_home();
  if (bundled_python.has_value()) {
    dbgln("🐍 Using bundled Python at: {}", bundled_python.value());
    wchar_t *python_home_wide =
        Py_DecodeLocale(bundled_python.value().characters(), nullptr);
    if (python_home_wide) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
      Py_SetPythonHome(python_home_wide);
#pragma clang diagnostic pop
      // Note: python_home_wide should NOT be freed - Py_SetPythonHome keeps the
      // pointer
    }
  } else {
    dbgln("🐍 Bundled Python not found, using system Python");
  }
#endif

  // Initialize Python interpreter
  Py_Initialize();

  debug_python_status("After Py_Initialize()");

  // Configure Python to use UTF-8 for stdout/stderr
  PyObject *sys_module = PyImport_ImportModule("sys");
  if (sys_module) {
    // Set unbuffered mode for immediate output
    PyObject *one = PyLong_FromLong(1);
    if (one) {
      PyObject_SetAttrString(sys_module, "dont_write_bytecode", one);
      Py_DECREF(one);
    }

    // Try to reconfigure stdout/stderr to UTF-8 (Python 3.7+)
    PyObject *stdout_obj = PyObject_GetAttrString(sys_module, "stdout");
    if (stdout_obj && stdout_obj != Py_None) {
      // Reconfigure to UTF-8 with error handling
      PyObject *reconfigure = PyObject_GetAttrString(stdout_obj, "reconfigure");
      if (reconfigure && PyCallable_Check(reconfigure)) {
        PyObject *kwargs = Py_BuildValue("{s:s,s:s}", "encoding", "utf-8",
                                         "errors", "replace");
        if (kwargs) {
          PyObject *result = PyObject_Call(reconfigure, PyTuple_New(0), kwargs);
          Py_XDECREF(result);
          Py_DECREF(kwargs);
        }
        Py_DECREF(reconfigure);
      }
      Py_DECREF(stdout_obj);
    }

    PyObject *stderr_obj = PyObject_GetAttrString(sys_module, "stderr");
    if (stderr_obj && stderr_obj != Py_None) {
      PyObject *reconfigure = PyObject_GetAttrString(stderr_obj, "reconfigure");
      if (reconfigure && PyCallable_Check(reconfigure)) {
        PyObject *kwargs = Py_BuildValue("{s:s,s:s}", "encoding", "utf-8",
                                         "errors", "replace");
        if (kwargs) {
          PyObject *result = PyObject_Call(reconfigure, PyTuple_New(0), kwargs);
          Py_XDECREF(result);
          Py_DECREF(kwargs);
        }
        Py_DECREF(reconfigure);
      }
      Py_DECREF(stderr_obj);
    }

    Py_DECREF(sys_module);
  }

  // GIL is automatically initialized in Python 3.9+

  dbgln("🐍 Initializing Python DOM API...");
  // Initialize Python DOM API
  Web::Bindings::PythonDOMAPI::initialize_module();

  dbgln("🐍 Initializing Python security model...");
  // Initialize Python security model
  (void)Web::HTML::PythonSecurityModel::initialize_security();

  // Set up restricted builtins in the global builtins module
  // This must be done before any scripts run to prevent KeyError during imports
  (void)Web::HTML::PythonSecurityModel::setup_global_restricted_builtins();

  s_initialized = true;

  dbgln("🐍 PythonEngine initialization complete!");
  debug_test_python_execution();
}

void PythonEngine::shutdown() {
  if (!s_initialized) {
    return;
  }

  PyGILState_Ensure(); // Acquire GIL before finalizing
  Py_Finalize();
  s_initialized = false;
}

bool PythonEngine::is_initialized() {
  dbgln("🐍 PythonEngine::is_initialized() = {}", s_initialized);
  return s_initialized;
}

void *PythonEngine::get_main_interpreter_state() {
  if (!s_initialized) {
    return nullptr;
  }

  // Note: This is a simplified approach. In a real implementation,
  // we'd need more sophisticated interpreter state management
  return PyGILState_GetThisThreadState();
}

void *PythonEngine::create_subinterpreter() {
  if (!s_initialized) {
    return nullptr;
  }

  PyGILState_STATE gstate = PyGILState_Ensure();

  // Create a new subinterpreter
  PyThreadState *subThreadState = Py_NewInterpreter();

  PyGILState_Release(gstate);

  return subThreadState;
}

void PythonEngine::destroy_subinterpreter(void *subinterpreter) {
  if (!s_initialized || !subinterpreter) {
    return;
  }

  PyGILState_STATE gstate = PyGILState_Ensure();

  PyThreadState *subThreadState = static_cast<PyThreadState *>(subinterpreter);

  // Restore thread state to main interpreter before deleting
  Py_EndInterpreter(subThreadState);

  PyGILState_Release(gstate);
}

} // namespace Web::HTML
