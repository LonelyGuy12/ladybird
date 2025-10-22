/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/HTML/Scripting/PythonPerformanceMetrics.h>
#include <AK/Time.h>
#include <Python.h>
#include <sys/time.h>
#include <sys/resource.h>

namespace Web::HTML {

// Static member initialization
PythonPerformanceMetrics::ExecutionStats PythonPerformanceMetrics::s_current_stats;
Optional<u64> PythonPerformanceMetrics::s_start_time;

// Start timing a Python execution
void PythonPerformanceMetrics::start_timing()
{
    s_start_time = AK::timestamp_now_us();
    
    // Reset execution stats
    s_current_stats.function_calls = 0;
    s_current_stats.gc_collections = 0;
    
    // Get initial memory usage
    update_memory_usage();
}

// End timing a Python execution and return stats
PythonPerformanceMetrics::ExecutionStats PythonPerformanceMetrics::end_timing()
{
    if (s_start_time.has_value()) {
        u64 end_time = AK::timestamp_now_us();
        s_current_stats.execution_time_ns = (end_time - s_start_time.value()) * 1000; // μs to ns
        s_start_time.clear();
        
        // Update final memory and CPU usage
        update_memory_usage();
        update_cpu_usage();
    }
    
    return s_current_stats;
}

// Record a Python function call
void PythonPerformanceMetrics::record_function_call()
{
    s_current_stats.function_calls++;
}

// Record a Python garbage collection cycle
void PythonPerformanceMetrics::record_gc_collection()
{
    s_current_stats.gc_collections++;
}

// Update memory usage statistics
void PythonPerformanceMetrics::update_memory_usage()
{
    // Use Python's memory allocation functions to get actual usage
    // This is a simplified version - in a real implementation we would use the Python C API
    // to get the actual memory usage of the Python interpreter
    
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        // This is just the process memory, not specifically Python's
        // In a real implementation we'd use PyObject_Size or the gc module
        s_current_stats.memory_usage_bytes = static_cast<u64>(usage.ru_maxrss) * 1024;
    }
    
    // For a more accurate measurement, we could use Python's sys.getsizeof
    // on various objects and track allocations/deallocations
}

// Update CPU usage statistics
void PythonPerformanceMetrics::update_cpu_usage()
{
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        // Calculate CPU time in seconds
        double user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;
        double sys_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;
        double total_time = user_time + sys_time;
        
        // Convert to percentage based on elapsed time
        if (s_start_time.has_value()) {
            u64 end_time = AK::timestamp_now_us();
            double elapsed = (end_time - s_start_time.value()) / 1000000.0; // μs to s
            if (elapsed > 0) {
                s_current_stats.cpu_usage_percent = (total_time / elapsed) * 100.0;
            }
        }
    }
}

// Get current performance statistics
PythonPerformanceMetrics::ExecutionStats PythonPerformanceMetrics::get_current_stats()
{
    return s_current_stats;
}

} // namespace Web::HTML
