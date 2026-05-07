cmake_minimum_required(VERSION 3.10)
project(printAscii C)

# Set C standard and enable required
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(ENABLE_PROFILING "Enable gprof profiling output" OFF)
if(ENABLE_PROFILING)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
endif()

# Main library and executable
add_subdirectory(src)

# Tests (only build if requested)
option(BUILD_TESTS "Build test suite" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

option(BUILD_BENCHMARKS "Build benchmarks suite" ON)
if(BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
