/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/JsonObject.h>
#include <AK/JsonValue.h>
#include <AK/String.h>
#include <AK/StringBuilder.h>
#include <AK/Time.h>
#include <LibJS/Runtime/Array.h>
#include <LibJS/Runtime/Error.h>
#include <LibJS/Runtime/Object.h>
#include <LibJS/Runtime/PrimitiveString.h>
#include <LibJS/Runtime/Value.h>
#include <LibWeb/Bindings/OptionalPythonJSBridge.h>

namespace Web::Bindings {

struct OptionalPythonJSBridge::Impl {
    // Python types forward declarations
    typedef struct _object PyObject;
    
    // Cache for converted objects to prevent circular references
    HashMap<void*, JS::Value> python_to_js_cache;
    HashMap<String, void*> js_to_python_cache;
    
    // Bridge mode
    enum class Mode {
        None,     // No bridge active
        MessagePassing // Message-based communication (safest)
    };
    
    Mode mode { Mode::None };

    // Message queue for asynchronous communication
    struct Message {
        String type;
        void* python_data { nullptr };
        JS::Value js_data;
        bool processed { false };
    };
    
    Vector<Message> message_queue;

    // Security settings
    bool allow_function_passing { false };
    bool allow_complex_objects { true };
    size_t max_serialization_depth { 5 };
    
    // Performance tracking
    u64 total_serialization_time_ns { 0 };
    u64 total_deserialization_time_ns { 0 };
    u64 serialization_count { 0 };
    u64 deserialization_count { 0 };
};

OptionalPythonJSBridge::OptionalPythonJSBridge()
    : m_impl(make<Impl>())
{
}

OptionalPythonJSBridge::~OptionalPythonJSBridge()
{
}

ErrorOr<NonnullOwnPtr<OptionalPythonJSBridge>> OptionalPythonJSBridge::create()
{
    auto bridge = make<OptionalPythonJSBridge>();
    TRY(bridge->initialize());
    return bridge;
}

ErrorOr<void> OptionalPythonJSBridge::initialize()
{
    if (m_initialized)
        return {};
        
    // Set up the bridge in message-passing mode
    m_impl->mode = Impl::Mode::MessagePassing;
    
    m_initialized = true;
    return {};
}

ErrorOr<void*> OptionalPythonJSBridge::send_to_js(void* python_data, StringView message_type)
{
    if (!m_initialized)
        return Error::from_string_literal("Bridge not initialized");
    
    m_stats.messages_to_js++;
    
    // Process Python data safely
    if (!TRY(is_deserialization_safe(python_data)))
        return Error::from_string_literal("Unsafe Python data for deserialization");
    
    // Measure deserialization time
    u64 start_time = AK::high_resolution_time().nanoseconds();
    
    // Convert Python data to JS
    auto js_value = TRY(deserialize_python_to_js(python_data));
    
    // Update performance stats
    u64 end_time = AK::high_resolution_time().nanoseconds();
    m_impl->total_deserialization_time_ns += (end_time - start_time);
    m_impl->deserialization_count++;
    m_stats.avg_deserialization_time_ms = 
        (double)m_impl->total_deserialization_time_ns / (1000.0 * 1000.0) / (double)m_impl->deserialization_count;
    
    // Find message handler
    String type_str = message_type.to_string();
    if (m_js_message_handlers.contains(type_str)) {
        // Handle the message with the registered handler
        auto result = m_js_message_handlers.get(type_str).value()(js_value);
        
        // Convert JS result back to Python
        return TRY(serialize_js_to_python(result));
    }
    
    // Add to message queue for async processing
    Impl::Message message;
    message.type = type_str;
    message.js_data = js_value;
    message.python_data = nullptr;
    message.processed = false;
    m_impl->message_queue.append(move(message));
    
    // Return null for async processing
    return nullptr;
}

ErrorOr<JS::Value> OptionalPythonJSBridge::send_to_python(JS::Value js_data, StringView message_type)
{
    if (!m_initialized)
        return Error::from_string_literal("Bridge not initialized");
    
    m_stats.messages_to_python++;
    
    // Process JS data safely
    if (!TRY(is_serialization_safe(js_data)))
        return Error::from_string_literal("Unsafe JS data for serialization");
    
    // Measure serialization time
    u64 start_time = AK::high_resolution_time().nanoseconds();
    
    // Convert JS data to Python
    auto* python_data = TRY(serialize_js_to_python(js_data));
    
    // Update performance stats
    u64 end_time = AK::high_resolution_time().nanoseconds();
    m_impl->total_serialization_time_ns += (end_time - start_time);
    m_impl->serialization_count++;
    m_stats.avg_serialization_time_ms = 
        (double)m_impl->total_serialization_time_ns / (1000.0 * 1000.0) / (double)m_impl->serialization_count;
    
    // Find message handler
    String type_str = message_type.to_string();
    if (m_python_message_handlers.contains(type_str)) {
        // Pass to Python handler
        void* handler = m_python_message_handlers.get(type_str).value();
        
        // In the actual implementation, we would call the Python handler here
        // For now, we just return a placeholder value
        return JS::js_undefined();
    }
    
    // Add to message queue for async processing
    Impl::Message message;
    message.type = type_str;
    message.python_data = python_data;
    message.js_data = js_data;
    message.processed = false;
    m_impl->message_queue.append(move(message));
    
    // Return undefined for async processing
    return JS::js_undefined();
}

ErrorOr<void> OptionalPythonJSBridge::register_js_message_handler(StringView message_type, Function<JS::Value(JS::Value)> handler)
{
    if (!m_initialized)
        return Error::from_string_literal("Bridge not initialized");
    
    m_js_message_handlers.set(message_type.to_string(), move(handler));
    return {};
}

ErrorOr<void> OptionalPythonJSBridge::register_python_message_handler(StringView message_type, void* handler)
{
    if (!m_initialized)
        return Error::from_string_literal("Bridge not initialized");
    
    if (!handler)
        return Error::from_string_literal("Invalid Python handler");
    
    m_python_message_handlers.set(message_type.to_string(), handler);
    return {};
}

ErrorOr<JS::Value> OptionalPythonJSBridge::deserialize_python_to_js(void* python_data)
{
    // Check cache first to handle circular references
    if (m_impl->python_to_js_cache.contains(python_data))
        return m_impl->python_to_js_cache.get(python_data).value();
    
    // In a real implementation, this would convert Python objects to JS
    // based on their type
    
    // For this simplified version, we just return undefined
    return JS::js_undefined();
}

ErrorOr<void*> OptionalPythonJSBridge::serialize_js_to_python(JS::Value js_value)
{
    // In a real implementation, this would convert JS values to Python objects
    // based on their type
    
    // For this simplified version, we just return nullptr
    return nullptr;
}

ErrorOr<void> OptionalPythonJSBridge::register_event_listener(StringView event_name, void* python_callback)
{
    if (!m_initialized)
        return Error::from_string_literal("Bridge not initialized");
    
    if (!python_callback)
        return Error::from_string_literal("Invalid Python callback");
    
    String name = event_name.to_string();
    
    if (!m_python_event_listeners.contains(name))
        m_python_event_listeners.set(name, {});
    
    m_python_event_listeners.get(name).value().append(python_callback);
    
    return {};
}

ErrorOr<void> OptionalPythonJSBridge::emit_event(StringView event_name, JS::Value event_data)
{
    if (!m_initialized)
        return Error::from_string_literal("Bridge not initialized");
    
    String name = event_name.to_string();
    
    if (!m_python_event_listeners.contains(name))
        return {};
    
    // Check if event data is safe to pass
    if (!TRY(is_serialization_safe(event_data)))
        return Error::from_string_literal("Unsafe event data");
    
    m_stats.events_triggered++;
    
    // Convert event data to Python
    auto* python_data = TRY(serialize_js_to_python(event_data));
    
    // Call all registered listeners
    auto& listeners = m_python_event_listeners.get(name).value();
    for (auto* listener : listeners) {
        // In a real implementation, we would call the Python callback here
        // For now, we just acknowledge it
    }
    
    return {};
}

ErrorOr<bool> OptionalPythonJSBridge::is_serialization_safe(JS::Value value)
{
    // Check for unsafe JS types that shouldn't be passed to Python
    auto type = TRY(determine_js_data_type(value));
    
    // Functions are only allowed if specifically enabled
    if (type == DataType::Function && !m_impl->allow_function_passing)
        return false;
    
    // Complex objects need recursive checks
    if (type == DataType::Object || type == DataType::Array) {
        // Implementation would check for circular references and max depth
        // For now, we just return the setting
        return m_impl->allow_complex_objects;
    }
    
    return true;
}

ErrorOr<bool> OptionalPythonJSBridge::is_deserialization_safe(void* python_data)
{
    if (!python_data)
        return true;
    
    // Check for unsafe Python types that shouldn't be passed to JS
    auto type = TRY(determine_python_data_type(python_data));
    
    // Functions are only allowed if specifically enabled
    if (type == DataType::Function && !m_impl->allow_function_passing)
        return false;
    
    // Complex objects need recursive checks
    if (type == DataType::Object || type == DataType::Array) {
        // Implementation would check for circular references and max depth
        // For now, we just return the setting
        return m_impl->allow_complex_objects;
    }
    
    return true;
}

ErrorOr<OptionalPythonJSBridge::DataType::Type> OptionalPythonJSBridge::determine_js_data_type(JS::Value value)
{
    if (value.is_null() || value.is_undefined())
        return DataType::Null;
    
    if (value.is_boolean())
        return DataType::Boolean;
    
    if (value.is_number())
        return DataType::Number;
    
    if (value.is_string())
        return DataType::String;
    
    if (value.is_array())
        return DataType::Array;
    
    if (value.is_object())
        return DataType::Object;
    
    if (value.is_function())
        return DataType::Function;
    
    if (value.is_error())
        return DataType::Error;
    
    return DataType::Unknown;
}

ErrorOr<OptionalPythonJSBridge::DataType::Type> OptionalPythonJSBridge::determine_python_data_type(void* python_data)
{
    // In a real implementation, this would use Python C API to determine types
    // For now, we just return Object as a placeholder
    return DataType::Object;
}

} // namespace Web::Bindings
