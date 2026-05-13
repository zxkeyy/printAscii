#ifndef CONFIG_H
#define CONFIG_H

#include "core/ascii_ramp.h"

typedef struct {
    char* play_cast_path;
    char* input_path;
    char* output_path;
    char* export_cast_path;
    int no_terminal_output;
    int video;
    int video_loop;
    float video_fps;
    int video_max_frames;
    int width;
    int height;
    int alpha;
    int negative;
    int color;
    int color_background_mode;
    char* tiling_text;
    int threshold;
    int threshold_value;
    int dither;
    int dither_threshold;
    int sobel_edge_detection;
    int sobel_edge_detection_threshold;
    int canny_edge_detection;
    float canny_edge_detection_sigma;
    int canny_edge_detection_high_threshold;
    int canny_edge_detection_low_threshold;
    int canny_auto_sigma;
    int canny_auto_high;
    int canny_auto_low;
    int braille;
    int halfblock;
    int verbose;
    char* debug_dir;
    float font_aspect_ratio; // Width to height ratio of the font
    AsciiRamp ramp;
} AppConfig;

AppConfig get_default_config();
void print_usage(const char* program_name);
int parse_arguments(int argc, char* argv[], AppConfig* config);
int validate_config(AppConfig* config);

#endif // CONFIG_H