/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Time.h>
#include <AK/Types.h>
#include <AK/Optional.h>

namespace Web::HTML {

// Performance metrics for Python execution
class PythonPerformanceMetrics {
public:
    struct ExecutionStats {
        u64 execution_time_ns { 0 };
        u64 memory_usage_bytes { 0 };
        u64 function_calls { 0 };
        u64 gc_collections { 0 };
        double cpu_usage_percent { 0.0 };
    };
    
    // Performance tracking methods
    static void start_timing();
    static ExecutionStats end_timing();
    static void record_function_call();
    static void record_gc_collection();
    static void update_memory_usage();
    static void update_cpu_usage();
    
    // Get current statistics
    static ExecutionStats get_current_stats();
    
private:
    static ExecutionStats s_current_stats;
    static Optional<MonotonicTime> s_start_time;
};

} // namespace Web::HTML
