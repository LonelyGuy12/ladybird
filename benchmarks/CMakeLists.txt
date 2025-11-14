include(FetchContent)

set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    google_benchmark
    GIT_REPOSITORY https://github.com/CodSpeedHQ/codspeed-cpp
    SOURCE_SUBDIR google_benchmark
    GIT_TAG main
)

FetchContent_MakeAvailable(google_benchmark)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

add_executable(python_benchmark python_benchmark.cpp)
target_link_libraries(python_benchmark
    PRIVATE
        benchmark::benchmark
        Python3::Python
)
