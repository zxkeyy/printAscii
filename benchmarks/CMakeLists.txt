set(BENCHMARK_SOURCES
    main.c
    benchmark.c
)

add_executable(run_benchmarks ${BENCHMARK_SOURCES})

# Target core library includes
target_include_directories(run_benchmarks PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external
)

# Disable profiling and force extreme optimization for true benchmarks
target_compile_options(run_benchmarks PRIVATE -O3 -fno-omit-frame-pointer)

# Strip out profiling flags -pg from the parent CMakeLists if inherited
string(REPLACE "-pg" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REPLACE "-pg" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

# Link against the core library
target_link_libraries(run_benchmarks PRIVATE printAsciiLib m)
