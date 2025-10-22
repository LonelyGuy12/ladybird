/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Error.h>
#include <AK/Function.h>
#include <AK/HashMap.h>
#include <AK/NonnullOwnPtr.h>
#include <AK/String.h>
#include <LibJS/Forward.h>
#include <LibWeb/Forward.h>

namespace Web::Bindings {

// Optional bridge for Python-JavaScript communication
// Only loaded when explicitly requested by the user
class OptionalPythonJSBridge {
public:
    static ErrorOr<NonnullOwnPtr<OptionalPythonJSBridge>> create();
    ~OptionalPythonJSBridge();

    // Initialize the bridge
    ErrorOr<void> initialize();

    // Check if the bridge is initialized
    bool is_initialized() const { return m_initialized; }

    // Message passing API (avoids direct object references)

    // Send a message from Python to JavaScript
    ErrorOr<void*> send_to_js(void* python_data, StringView message_type);

    // Send a message from JavaScript to Python
    ErrorOr<JS::Value> send_to_python(JS::Value js_data, StringView message_type);

    // Register message handlers
    ErrorOr<void> register_js_message_handler(StringView message_type, Function<JS::Value(JS::Value)> handler);
    ErrorOr<void> register_python_message_handler(StringView message_type, void* handler);

    // Data conversion helpers
    struct DataType {
        enum Type {
            Null,
            Boolean,
            Number,
            String,
            Array,
            Object,
            Function,
            Error,
            Unknown
        };
    };

    // Convert data between Python and JavaScript with serialization
    ErrorOr<JS::Value> deserialize_python_to_js(void* python_data);
    ErrorOr<void*> serialize_js_to_python(JS::Value js_value);

    // Event handling
    ErrorOr<void> register_event_listener(StringView event_name, void* python_callback);
    ErrorOr<void> emit_event(StringView event_name, JS::Value event_data);

    // Get statistics about bridge usage
    struct BridgeStats {
        u64 messages_to_js { 0 };
        u64 messages_to_python { 0 };
        u64 serialization_errors { 0 };
        u64 events_triggered { 0 };
        double avg_serialization_time_ms { 0.0 };
        double avg_deserialization_time_ms { 0.0 };
    };

    BridgeStats get_stats() const { return m_stats; }

private:
    OptionalPythonJSBridge();

    bool m_initialized { false };
    
    struct Impl;
    NonnullOwnPtr<Impl> m_impl;

    BridgeStats m_stats;

    // Message handlers
    HashMap<String, Function<JS::Value(JS::Value)>> m_js_message_handlers;
    HashMap<String, void*> m_python_message_handlers;

    // Event listeners
    HashMap<String, Vector<void*>> m_python_event_listeners;

    // Security checks
    ErrorOr<bool> is_serialization_safe(JS::Value value);
    ErrorOr<bool> is_deserialization_safe(void* python_data);

    // Data validation
    ErrorOr<DataType::Type> determine_js_data_type(JS::Value value);
    ErrorOr<DataType::Type> determine_python_data_type(void* python_data);
};

} // namespace Web::Bindings
