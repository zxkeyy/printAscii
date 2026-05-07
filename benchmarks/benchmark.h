#ifndef BENCHMARK_H
#define BENCHMARK_H

// Ensure POSIX time functions are available
#define _POSIX_C_SOURCE 199309L

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

typedef struct {
    const char* name;
    void* (*setup)(void);
    void (*run)(void* state);
    void (*teardown)(void* state);
    int iterations;
} Benchmark;

void run_benchmark(Benchmark* bench);

#endif
