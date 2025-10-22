/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/HashMap.h>
#include <AK/String.h>
#include <AK/Function.h>
#include <LibWeb/Forward.h>

namespace Web::Python {

// Python Standard Library for Browser Environment
class PythonStdLib {
public:
    static ErrorOr<NonnullOwnPtr<PythonStdLib>> create();

    // Initialize all standard library modules
    ErrorOr<void> initialize();

    // Get module by name
    void* get_module(StringView name);

    // Available modules
    void* asyncio_module();
    void* json_module();
    void* urllib_module();
    void* os_module();
    void* sys_module();
    void* time_module();
    void* threading_module();
    void* collections_module();
    void* itertools_module();
    void* functools_module();
    void* re_module();
    void* math_module();
    void* random_module();
    void* hashlib_module();
    void* base64_module();
    void* datetime_module();

    // Browser-specific modules
    void* browser_module();
    void* dom_module();
    void* webapi_module();

private:
    PythonStdLib() = default;

    struct ModuleInfo {
        void* module_dict { nullptr };
        Function<ErrorOr<void>(void*)> initializer;
        bool initialized { false };
    };

    HashMap<String, ModuleInfo> m_modules;
    void* m_main_namespace { nullptr };

    // Module initialization functions
    ErrorOr<void> init_asyncio(void* module_dict);
    ErrorOr<void> init_json(void* module_dict);
    ErrorOr<void> init_urllib(void* module_dict);
    ErrorOr<void> init_os(void* module_dict);
    ErrorOr<void> init_sys(void* module_dict);
    ErrorOr<void> init_time(void* module_dict);
    ErrorOr<void> init_threading(void* module_dict);
    ErrorOr<void> init_collections(void* module_dict);
    ErrorOr<void> init_itertools(void* module_dict);
    ErrorOr<void> init_functools(void* module_dict);
    ErrorOr<void> init_re(void* module_dict);
    ErrorOr<void> init_math(void* module_dict);
    ErrorOr<void> init_random(void* module_dict);
    ErrorOr<void> init_hashlib(void* module_dict);
    ErrorOr<void> init_base64(void* module_dict);
    ErrorOr<void> init_datetime(void* module_dict);

    // Browser modules
    ErrorOr<void> init_browser(void* module_dict);
    ErrorOr<void> init_dom(void* module_dict);
    ErrorOr<void> init_webapi(void* module_dict);

    // Utility functions for module creation
    ErrorOr<void> add_function_to_module(void* module_dict, String const& name, void* function);
    ErrorOr<void> add_constant_to_module(void* module_dict, String const& name, void* value);
    ErrorOr<void> add_class_to_module(void* module_dict, String const& name, void* type_object);
};

} // namespace Web::Python
