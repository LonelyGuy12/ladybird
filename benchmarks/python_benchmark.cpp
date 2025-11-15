#include <benchmark/benchmark.h>
// Benchmark CPython execution using embedded interpreter
#include <Python.h>

static void BM_PythonFibonacci(benchmark::State& state)
{
    if (!Py_IsInitialized())
        Py_Initialize();

    const char* python_code = R"(
def fib(n):
    return n if n < 2 else fib(n-1) + fib(n-2)
result = fib(20)
)";

    for (auto _ : state) {
        // Execute the Python code in the embedded interpreter
        PyRun_SimpleString(python_code);
    }
}

BENCHMARK(BM_PythonFibonacci);

BENCHMARK_MAIN();
