#include <AK/Error.h>
#include <AK/String.h>
#include <AK/Optional.h>

namespace Web::HTML {

class PythonError : public Error {
public:
    static ErrorOr<String> format_python_exception();
    static Error from_python_exception();
    
    PythonError(String message, String type, String traceback)
        : Error(Error::from_errno(EFAULT))
        , m_message(move(message))
        , m_type(move(type))
        , m_traceback(move(traceback))
    {
    }
    
    String const& message() const { return m_message; }
    String const& type() const { return m_type; }
    String const& traceback() const { return m_traceback; }
    
    virtual StringView string_literal() const override { return m_message.view(); }
    
private:
    String m_message;
    String m_type;
    String m_traceback;
};

// Performance utilities for Python execution
class PythonPerformanceMetrics {
public:
    struct ExecutionStats {
        u64 execution_time_ns { 0 };
        u64 memory_usage_bytes { 0 };
        u64 function_calls { 0 };
        u64 gc_collections { 0 };
        double cpu_usage_percent { 0.0 };
    };
    
    static void start_timing();
    static ExecutionStats end_timing();
    static void record_function_call();
    static void record_gc_collection();
    static void update_memory_usage();
    static void update_cpu_usage();
    
    static ExecutionStats get_current_stats();
    
private:
    static ExecutionStats s_current_stats;
    static Optional<u64> s_start_time;
};

} // namespace Web::HTML
