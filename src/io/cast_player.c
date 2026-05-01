#include "io/cast_player.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

static volatile sig_atomic_t g_player_interrupted = 0;

static void player_interrupt_handler(int sig) {
    (void)sig;
    g_player_interrupted = 1;
}

int cast_player_play(const char* path) {
    if (!path) return -1;

    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Failed to open cast file: %s\n", path);
        return -1;
    }

    signal(SIGINT, player_interrupt_handler);
    signal(SIGTERM, player_interrupt_handler);

    printf("\033[2J\033[H\033[?25l");
    fflush(stdout);

    // Provide a sufficiently large buffer for terminal sequences frames
    char line[1048576]; 
    double last_time = 0.0;
    int first_frame = 1;

    // Read skipping the first header line or lines that don't look like array chunks
    while (fgets(line, sizeof(line), f) && !g_player_interrupted) {
        if (line[0] != '[') continue;

        char* p = line + 1;
        double t = strtod(p, &p);

        // Skip to output type
        while (*p == ' ' || *p == ',') p++;
        char type[16] = {0};
        if (*p == '"') {
            p++;
            int i = 0;
            while (*p && *p != '"' && i < 15) type[i++] = *p++;
            type[i] = '\0';
            if (*p == '"') p++;
        }
        
        // We only render "o" (output) lines
        if (strcmp(type, "o") != 0) continue;

        // Skip to data string
        while (*p == ' ' || *p == ',') p++;
        
        if (*p == '"') {
            p++;
            size_t len = strlen(p);
            char* out = malloc(len + 1);
            int out_idx = 0;
            
            while (*p) {
                if (*p == '\\') {
                    p++;
                    if (*p == 'n') out[out_idx++] = '\n';
                    else if (*p == 'r') out[out_idx++] = '\r';
                    else if (*p == 't') out[out_idx++] = '\t';
                    else if (*p == '\\') out[out_idx++] = '\\';
                    else if (*p == '"') out[out_idx++] = '"';
                    else if (*p == 'e') out[out_idx++] = '\033'; 
                    else if (*p == 'u') {
                        unsigned int codepoint = 0;
                        if (sscanf(p + 1, "%04x", &codepoint) == 1) {
                            out[out_idx++] = (char)codepoint;
                            p += 4;
                        }
                    } else {
                        out[out_idx++] = *p;
                    }
                } else if (*p == '"') {
                    break;
                } else {
                    out[out_idx++] = *p;
                }
                p++;
            }
            out[out_idx] = '\0';

            // Sleep calculation
            if (!first_frame && !g_player_interrupted) {
                double delay = t - last_time;
                if (delay > 0.0) {
                    struct timespec sleep_time;
                    sleep_time.tv_sec = (time_t)delay;
                    sleep_time.tv_nsec = (long)((delay - sleep_time.tv_sec) * 1000000000.0);
                    nanosleep(&sleep_time, NULL);
                }
            }
            
            if (!g_player_interrupted) {
                printf("%s", out);
                fflush(stdout);
            }
            
            first_frame = 0;
            last_time = t;
            free(out);
        }
    }

    fclose(f);
    printf("\033[?25h\n");
    fflush(stdout);

    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);

    return 0;
}
