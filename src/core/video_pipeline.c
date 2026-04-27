#include "core/video_pipeline.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "core/image.h"
#include "core/image_pipeline.h"
#include "io/video_loader.h"

static volatile sig_atomic_t g_video_interrupted = 0;
static void (*g_old_sigint_handler)(int) = SIG_DFL;
static void (*g_old_sigterm_handler)(int) = SIG_DFL;
static volatile sig_atomic_t g_signal_handlers_installed = 0;

static void restore_cursor_stdout(void) {
    const char* show_cursor = "\033[?25h";
    write(STDOUT_FILENO, show_cursor, 6);
}

static void video_interrupt_handler(int signal_number) {
    (void)signal_number;
    g_video_interrupted = 1;
    restore_cursor_stdout();
}

static void install_video_signal_handlers(void) {
    g_old_sigint_handler = signal(SIGINT, video_interrupt_handler);
    g_old_sigterm_handler = signal(SIGTERM, video_interrupt_handler);

    if (g_old_sigint_handler != SIG_ERR && g_old_sigterm_handler != SIG_ERR) {
        g_signal_handlers_installed = 1;
        return;
    }

    signal(SIGINT, g_old_sigint_handler);
    signal(SIGTERM, g_old_sigterm_handler);
}

static void restore_video_signal_handlers(void) {
    if (!g_signal_handlers_installed) {
        return;
    }

    signal(SIGINT, g_old_sigint_handler);
    signal(SIGTERM, g_old_sigterm_handler);
    g_signal_handlers_installed = 0;
}

static int get_timestamp(struct timespec* ts) {
    if (!ts) {
        return -1;
    }

#if defined(CLOCK_MONOTONIC)
    if (clock_gettime(CLOCK_MONOTONIC, ts) == 0) {
        return 0;
    }
#endif

    return (timespec_get(ts, TIME_UTC) == TIME_UTC) ? 0 : -1;
}

static double timespec_diff_seconds(const struct timespec* start, const struct timespec* end) {
    double sec = (double)(end->tv_sec - start->tv_sec);
    double nsec = (double)(end->tv_nsec - start->tv_nsec) / 1000000000.0;
    return sec + nsec;
}

int video_pipeline_run_terminal(const AppConfig* config) {
    if (!config) {
        fprintf(stderr, "Video pipeline received NULL config\n");
        return EXIT_FAILURE;
    }

    g_video_interrupted = 0;
    install_video_signal_handlers();

    printf("\033[2J\033[H\033[?25l");
    fflush(stdout);

    int exit_code = EXIT_SUCCESS;
    int processed_frames = 0;

    while (!g_video_interrupted) {
        VideoStream stream;
        if (video_stream_open(&stream, config->input_path, config->video_fps) != 0) {
            fprintf(stderr, "Failed to open video stream\n");
            exit_code = EXIT_FAILURE;
            break;
        }

        const double frame_duration_seconds = (stream.fps > 0.0f) ? (1.0 / (double)stream.fps) : (1.0 / 24.0);
        int reached_eof = 0;
        int stream_error = 0;

        // Process all frames from current stream
        while (!g_video_interrupted && !stream_error) {
            struct timespec frame_start = {0};
            int has_timing_start = (get_timestamp(&frame_start) == 0);

            Image* frame = NULL;
            int read_status = video_stream_read_frame(&stream, &frame);
            
            // End of file - normal completion
            if (read_status == 0) {
                reached_eof = 1;
                break;
            }

            // Frame read error
            if (read_status < 0 || !frame) {
                fprintf(stderr, "Failed to decode video frame %d\n", processed_frames + 1);
                exit_code = EXIT_FAILURE;
                stream_error = 1;
                break;
            }

            printf("\033[H");
            ProcessingResult* result = image_pipeline_process(frame, config);

            if (!result) {
                fprintf(stderr, "Failed to create processing result for frame %d\n", processed_frames + 1);
                image_free(frame);
                exit_code = EXIT_FAILURE;
                stream_error = 1;
                break;
            }

            if (result->status != PIPELINE_SUCCESS) {
                fprintf(stderr, "Frame %d processing failed: %s\n",
                        processed_frames + 1,
                        result->error_message ? result->error_message : "Unknown error");
                pipeline_free_result(result);
                image_free(frame);
                exit_code = EXIT_FAILURE;
                stream_error = 1;
                break;
            }

            pipeline_free_result(result);
            image_free(frame);
            processed_frames++;
            fflush(stdout);

            // Check frame limit
            if (config->video_max_frames > 0 && processed_frames >= config->video_max_frames) {
                break;
            }

            // Maintain playback speed
            if (has_timing_start) {
                struct timespec frame_end = {0};
                if (get_timestamp(&frame_end) == 0) {
                    double processing_time = timespec_diff_seconds(&frame_start, &frame_end);
                    double remaining = frame_duration_seconds - processing_time;

                    if (remaining > 0.0) {
                        struct timespec sleep_time;
                        sleep_time.tv_sec = (time_t)remaining;
                        sleep_time.tv_nsec = (long)((remaining - (double)sleep_time.tv_sec) * 1000000000.0);
                        nanosleep(&sleep_time, NULL);
                    }
                }
            }
        }

        // Always close stream, even on errors
        video_stream_close(&stream);

        // Exit on error or user interrupt
        if (exit_code != EXIT_SUCCESS || g_video_interrupted) {
            break;
        }

        // Exit if frame limit reached
        if (config->video_max_frames > 0 && processed_frames >= config->video_max_frames) {
            break;
        }

        // Exit if looping disabled or EOF not reached
        if (!config->video_loop || !reached_eof) {
            break;
        }
    }

    restore_video_signal_handlers();
    printf("\033[?25h\n");
    fflush(stdout);
    return exit_code;
}