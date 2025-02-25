#ifndef CONFIG_H
#define CONFIG_H

#include "core/ascii_ramp.h"

typedef struct {
    char* input_path;
    char* output_path;
    int no_terminal_output;
    int width;
    int height;
    int alpha;
    int negative;
    int dither;
    int dither_threshold;
    int sobel_edge_detection;
    int sobel_edge_detection_threshold;
    int canny_edge_detection;
    float canny_edge_detection_sigma;
    int canny_edge_detection_high_threshold;
    int canny_edge_detection_low_threshold;
    int verbose;
    float font_aspect_ratio; // Width to height ratio of the font
    AsciiRamp ramp;
} AppConfig;

void print_usage(const char* program_name);
int parse_arguments(int argc, char* argv[], AppConfig* config);
int validate_config(AppConfig* config);

#endif // CONFIG_H