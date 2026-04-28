#include "core/video_pipeline.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

#include "core/image.h"
#include "core/image_pipeline.h"
#include "io/video_loader.h"

static volatile sig_atomic_t g_video_interrupted = 0;
static void (*g_old_sigint_handler)(int) = SIG_DFL;
static void (*g_old_sigterm_handler)(int) = SIG_DFL;
static volatile sig_atomic_t g_signal_handlers_installed = 0;

static struct termios g_old_termios;
static volatile sig_atomic_t g_terminal_state_saved = 0;

static void disable_raw_mode(void) {
    if (!g_terminal_state_saved) return;
    
    tcsetattr(STDIN_FILENO, TCSANOW, &g_old_termios);
    g_terminal_state_saved = 0;
}

static void enable_raw_mode(void) {
    if (!isatty(STDIN_FILENO)) return;
    
    if (tcgetattr(STDIN_FILENO, &g_old_termios) == 0) {
        struct termios raw = g_old_termios;
        raw.c_lflag &= ~(ECHO | ICANON);
        
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        g_terminal_state_saved = 1;
        atexit(disable_raw_mode);
    }
}

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
    enable_raw_mode();

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
        int is_paused = 0;

        // Process all frames from current stream
        while (!g_video_interrupted && !stream_error) {
            int advance_frame = !is_paused;

            // Handle interactive input — drain all buffered keypresses, but only
            // honour one space toggle per frame cycle to prevent double-toggling
            // when two spaces arrive in the same batch.
            int space_seen = 0;
            while (!g_video_interrupted) {
                fd_set read_fds;
                FD_ZERO(&read_fds);
                FD_SET(STDIN_FILENO, &read_fds);
                struct timeval timeout = {0, 0};
                
                if (select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout) != 1) {
                    break;
                }

                char c;
                if (read(STDIN_FILENO, &c, 1) != 1) {
                    break;
                }

                if (c == 'q' || c == 'Q' || c == 27) { // 27 is ESC
                    g_video_interrupted = 1;
                    advance_frame = 0;
                } else if (c == ' ' && !space_seen) {
                    space_seen = 1;
                    is_paused = !is_paused;
                    advance_frame = !is_paused; // one frame on unpause
                } else if ((c == '.' || c == '>') && is_paused) {
                    advance_frame = 1; // frame step
                }
            }

            if (g_video_interrupted) break;

            if (!advance_frame) {
                struct timespec pause_sleep;
                pause_sleep.tv_sec = 0;
                pause_sleep.tv_nsec = 10000000; // 10ms CPU sleep while paused
                nanosleep(&pause_sleep, NULL);
                continue;
            }

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
    disable_raw_mode();
    printf("\033[?25h\n");
    fflush(stdout);
    return exit_code;
}