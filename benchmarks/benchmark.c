#include "benchmark.h"

static double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void run_benchmark(Benchmark* bench) {
    if (!bench || !bench->run) return;
    
    printf("Running benchmark: %s (%d iterations)...\n", bench->name, bench->iterations);
    
    void* state = NULL;
    if (bench->setup) {
        state = bench->setup();
    }

    // Warmup round: caches the instructions and branch predictions
    bench->run(state);

    double start_time = get_time();
    for (int i = 0; i < bench->iterations; i++) {
        bench->run(state);
    }
    double end_time = get_time();

    if (bench->teardown) {
        bench->teardown(state);
    }

    double total_time = end_time - start_time;
    double avg_time_ms = (total_time / bench->iterations) * 1000.0;
    
    printf("  Total time: %.4f s\n", total_time);
    printf("  Avg time/iter: %.4f ms\n", avg_time_ms);
    printf("  Theoretical Peak Iterations/sec: %.2f\n\n", 1.0 / (total_time / bench->iterations));
}
