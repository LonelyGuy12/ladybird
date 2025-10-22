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
#include <LibJS/Forward.h>

namespace Web::Bindings {

// Python Web Module provides browser APIs directly to Python
class PythonWebModule {
public:
    static ErrorOr<NonnullOwnPtr<PythonWebModule>> create();

    // Initialize the module with browser context
    ErrorOr<void> initialize_with_context(JS::Realm& realm);

    // Get module dictionary for Python import
    void* get_module_dict();

    // Browser API functions
    static void* create_dom_element(String const& tag_name);
    static void* get_element_by_id(String const& id);
    static void* query_selector(String const& selector);
    static void set_element_text(void* element, String const& text);
    static String get_element_text(void* element);
    static void set_element_attribute(void* element, String const& name, String const& value);
    static String get_element_attribute(void* element, String const& name);

    // Fetch API
    static void* fetch(String const& url, HashMap<String, String> const& headers = {});
    static String fetch_text(void* request);
    static void* fetch_json(void* request);

    // Storage API
    static void set_local_storage(String const& key, String const& value);
    static String get_local_storage(String const& key);
    static void remove_local_storage(String const& key);

    // Console API
    static void console_log(String const& message);
    static void console_error(String const& message);
    static void console_warn(String const& message);

    // Performance API
    static double performance_now();
    static HashMap<String, double> get_performance_timing();

    // Async utilities
    static void* create_promise();
    static void resolve_promise(void* promise, void* value);
    static void reject_promise(void* promise, String const& error);

    // Event handling
    static void add_event_listener(void* element, String const& event, Function<void()> handler);
    static void remove_event_listener(void* element, String const& event);

    // Timer functions
    static void set_timeout(Function<void()> callback, u32 delay_ms);
    static void set_interval(Function<void()> callback, u32 interval_ms);
    static void clear_timeout(u32 timer_id);
    static void clear_interval(u32 timer_id);

    // Network utilities
    static String get_current_url();
    static String get_user_agent();
    static HashMap<String, String> get_headers();

    // Security and sandboxing
    static bool is_secure_context();
    static String get_origin();
    static bool has_permission(String const& permission);

private:
    PythonWebModule() = default;

    struct Impl;
    NonnullOwnPtr<Impl> m_impl;

    // Browser context
    GC::Ptr<JS::Realm> m_realm;

    // Module objects
    void* m_module_dict { nullptr };
    void* m_browser_api { nullptr };
    void* m_dom_api { nullptr };
    void* m_fetch_api { nullptr };
    void* m_storage_api { nullptr };

    // Performance tracking
    HashMap<String, double> m_api_call_times;
    u64 m_total_api_calls { 0 };
};

} // namespace Web::Bindings