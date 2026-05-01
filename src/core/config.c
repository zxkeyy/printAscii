#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "core/config.h"

const AsciiRamp DEFAULT_RAMP = {
    .characters = " .:-=+*#@&8B$@",
    .length = 14
};

const AppConfig DEFAULT_CONFIG = {
    .play_cast_path = NULL,
    .input_path = NULL,
    .output_path = NULL,
    .export_cast_path = NULL,
    .no_terminal_output = 0,
    .video = 0,
    .video_loop = 0,
    .video_fps = 0.0f,
    .video_max_frames = 0,
    .width = 100,
    .height = 0,
    .alpha = 0,
    .negative = 0,
    .color = 0,
    .color_background_mode = 0,
    .tiling_text = "0",
    .threshold = 0,
    .threshold_value = 128,
    .dither = 0,
    .dither_threshold = 128,
    .sobel_edge_detection = 0,
    .sobel_edge_detection_threshold = 128,
    .canny_edge_detection = 0,
    .canny_edge_detection_sigma = 0.8,
    .canny_edge_detection_high_threshold = 120,
    .canny_edge_detection_low_threshold = 50,
    .braille = 0,
    .halfblock = 0,
    .font_aspect_ratio = 0.45,
    .verbose = 0,
    .debug_dir = ".",
    .ramp = {0} // Initialize all to zero and set it later in get function, because C ¯\_(ツ)_/¯.
};

AppConfig get_default_config() {
    AppConfig config = DEFAULT_CONFIG;
    // Copy the default ramp
    config.ramp = DEFAULT_RAMP;

    return config;
}

typedef enum {
    OUTPUT_MODE_UNSET = 0,
    OUTPUT_MODE_ASCII,
    OUTPUT_MODE_BRAILLE,
    OUTPUT_MODE_ANSI,
    OUTPUT_MODE_ANSI_BG,
    OUTPUT_MODE_HALFBLOCK
} OutputModeSelection;

typedef enum {
    EDGE_MODE_UNSET = 0,
    EDGE_MODE_NONE,
    EDGE_MODE_SOBEL,
    EDGE_MODE_CANNY
} EdgeModeSelection;

typedef struct {
    const char* name;
    const char* description;
    OutputModeSelection output_mode;
    EdgeModeSelection edge_mode;
    int threshold_enabled;
    int threshold_value;
    int dither_enabled;
    int dither_threshold;
    int invert_enabled;
    int sobel_threshold;
    float canny_sigma;
    int canny_high;
    int canny_low;
} PresetDefinition;

static const PresetDefinition PRESETS[] = {
    {
        .name = "photo",
        .description = "ASCII tuned for photos with mild dithering",
        .output_mode = OUTPUT_MODE_ASCII,
        .edge_mode = EDGE_MODE_NONE,
        .threshold_enabled = 0,
        .threshold_value = 128,
        .dither_enabled = 1,
        .dither_threshold = 110,
        .invert_enabled = 0,
        .sobel_threshold = 128,
        .canny_sigma = 0.8f,
        .canny_high = 120,
        .canny_low = 50
    },
    {
        .name = "lineart",
        .description = "High-contrast edge-focused output",
        .output_mode = OUTPUT_MODE_ASCII,
        .edge_mode = EDGE_MODE_SOBEL,
        .threshold_enabled = 1,
        .threshold_value = 135,
        .dither_enabled = 0,
        .dither_threshold = 128,
        .invert_enabled = 0,
        .sobel_threshold = 145,
        .canny_sigma = 0.8f,
        .canny_high = 120,
        .canny_low = 50
    },
    {
        .name = "terminal",
        .description = "Balanced default for terminal readability",
        .output_mode = OUTPUT_MODE_ASCII,
        .edge_mode = EDGE_MODE_NONE,
        .threshold_enabled = 0,
        .threshold_value = 128,
        .dither_enabled = 0,
        .dither_threshold = 128,
        .invert_enabled = 0,
        .sobel_threshold = 128,
        .canny_sigma = 0.8f,
        .canny_high = 120,
        .canny_low = 50
    }
};

static const size_t PRESET_COUNT = sizeof(PRESETS) / sizeof(PRESETS[0]);

static const PresetDefinition* find_preset(const char* preset_name) {
    if (!preset_name) {
        return NULL;
    }

    for (size_t i = 0; i < PRESET_COUNT; i++) {
        if (strcmp(PRESETS[i].name, preset_name) == 0) {
            return &PRESETS[i];
        }
    }

    return NULL;
}

static void apply_preset(const PresetDefinition* preset,
                         AppConfig* config,
                         OutputModeSelection* selected_output_mode,
                         EdgeModeSelection* selected_edge_mode) {
    if (!preset || !config || !selected_output_mode || !selected_edge_mode) {
        return;
    }

    *selected_output_mode = preset->output_mode;
    *selected_edge_mode = preset->edge_mode;

    config->threshold = preset->threshold_enabled;
    config->threshold_value = preset->threshold_value;
    config->dither = preset->dither_enabled;
    config->dither_threshold = preset->dither_threshold;
    config->negative = preset->invert_enabled;
    config->sobel_edge_detection_threshold = preset->sobel_threshold;
    config->canny_edge_detection_sigma = preset->canny_sigma;
    config->canny_edge_detection_high_threshold = preset->canny_high;
    config->canny_edge_detection_low_threshold = preset->canny_low;
}

static void print_available_presets(void) {
    printf("Presets:\n");
    for (size_t i = 0; i < PRESET_COUNT; i++) {
        printf("      %-12s %s\n", PRESETS[i].name, PRESETS[i].description);
    }
}


struct option long_options[] = {
    {"play-cast", required_argument, NULL, 'p'},
    {"input", required_argument, NULL, 'i'},
    {"output", required_argument, NULL, 'o'},
    {"export-cast", required_argument, NULL, 'E'},
    {"no-terminal-output", no_argument, NULL, 'q'},
    {"fit-terminal", no_argument, NULL, 'F'},
    {"fit-terminal-w", no_argument, NULL, 'W'},
    {"fit-terminal-h", no_argument, NULL, 'H'},
    {"video", no_argument, NULL, 'x'},
    {"video-loop", no_argument, NULL, 'L'},
    {"video-fps", required_argument, NULL, 8},
    {"video-max-frames", required_argument, NULL, 9},
    {"preset", required_argument, NULL, 'P'},
    {"ascii-gradient", required_argument, NULL, 'g'},
    {"mode", required_argument, NULL, 'm'},
    {"width", required_argument, NULL, 'w'},
    {"height", required_argument, NULL, 'h'},
    {"alpha", required_argument, NULL, 'a'},
    {"threshold", required_argument, NULL, 't'},
    {"invert", no_argument, NULL, 'n'},
    {"negative", no_argument, NULL, 'n'},
    {"dither", required_argument, NULL, 'd'},
    {"edge", required_argument, NULL, 'e'},
    {"edge-threshold", required_argument, NULL, 3},
    {"canny-sigma", required_argument, NULL, 4},
    {"canny-high", required_argument, NULL, 5},
    {"canny-low", required_argument, NULL, 6},
    {"font-aspect-ratio", required_argument, NULL, 'r'},
    {"color", no_argument, NULL, 'c'},
    {"color-background", no_argument, NULL, 'B'},
    {"braille", no_argument, NULL, 'b'},
    {"sobel-edge-detection", optional_argument, NULL, 's'},
    {"canny-edge-detection", optional_argument, NULL, 'C'},
    {"tiling-text", required_argument, NULL, 'T'},
    {"debug-dir", required_argument, NULL, 7},
    {"verbose", no_argument, NULL, 'v'},
    {"version", no_argument, NULL, 'V'},
    {"help", no_argument, NULL, '?'},
    {NULL, 0, NULL, 0}
};

void print_usage(const char* program_name) {
    printf("Usage: %s -i <input> [options]\n", program_name);
    printf("   or: %s -p <cast_file>\n\n", program_name);
    printf("Image to ASCII/ANSI/Unicode Art Converter\n\n");
    
    printf("Input/Output Options:\n");
    printf("  -p, --play-cast <file>     Play an Asciinema v2 (.cast) file directly in terminal\n");
    printf("  -i, --input <file>         Input image file (JPG, PNG, TGA, BMP, etc.)\n");
    printf("  -o, --output <file>        Output file (default: print to terminal)\n");
    printf("  -E, --export-cast <file>   Export video playback to an Asciinema v2 (.cast) file\n");
    printf("  -q, --no-terminal-output   Don't print the result to terminal\n");
    printf("  -F, --fit-terminal         Automatically size output to fit the terminal window\n");
    printf("  -W, --fit-terminal-w       Automatically size output to fit terminal width (keep aspect ratio)\n");
    printf("  -H, --fit-terminal-h       Automatically size output to fit terminal height (keep aspect ratio)\n\n");
    printf("  -x, --video                Treat input as video/GIF and render frames to terminal\n");
    printf("  -L, --video-loop           Loop video playback until interrupted\n");
    printf("      --video-fps <f>        Playback FPS override for video mode (default: source FPS)\n\n");
    printf("      --video-max-frames <n> Stop after processing N frames (default: unlimited)\n\n");

    printf("Preset Options:\n");
    printf("  -P, --preset <name>        Apply a processing preset (override with explicit flags)\n");
    print_available_presets();
    printf("\n");
    
    printf("Display Options:\n");
    printf("  -g, --ascii-gradient <str> ASCII gradient (default: ' .:-=+*#@&8B$@')\n");
    printf("  -m, --mode <type>          Output mode: ascii, braille, ansi, ansi-bg, halfblock (default: ascii)\n");
    printf("  -w, --width <n>            Output width in characters (default: 100)\n");
    printf("                             if only height is specified, width will be calculated to keep image aspect ratio\n");
    printf("                             Use -1 to keep source image width\n");
    printf("  -h, --height <n>           Output height in characters\n");
    printf("                             if only width is specified, height will be calculated to keep image aspect ratio\n");
    printf("                             Use -1 to keep source image height\n");
    printf("  -T, --tiling_text <str>    A string that will be used to tile the output if color is used (default: '0')\n");
    printf("  -r, --font-aspect-ratio <f> Font width to height ratio (default: 0.45)\n");
    printf("                             Change this if the output aspect ratio is incorrect\n\n");
    
    printf("Processing Options:\n");
    printf("  -a, --alpha <0-255>        Background brightness for transparency (default: 0)\n");
    printf("  -t, --threshold <0-255>    Threshold for black/white output (default: 128)\n");
    printf("  -n, --invert               Invert colors\n");
    printf("  -d, --dither <0-255>       Apply Floyd-Steinberg dithering (default: 128)\n\n");
    
    printf("Edge Detection:\n");
    printf("  -e, --edge <type>          Edge mode: none, sobel, canny (default: none)\n");
    printf("      --edge-threshold <n>   Sobel edge threshold (default: 128)\n");
    printf("      --canny-sigma <f>      Canny sigma (default: 0.8)\n");
    printf("      --canny-high <n>       Canny high threshold (default: 120)\n");
    printf("      --canny-low <n>        Canny low threshold (default: 50)\n\n");
    
    printf("General Options:\n");
    printf("      --debug-dir <dir>      Directory for verbose debug artifacts (default: .)\n");
    printf("  -v, --verbose              Display processing information\n");
    printf("  -V, --version              Show version information\n");
    printf("  --help                     Display this help message\n\n");
    
    printf("Examples:\n");
    printf("  %s -p video.cast                     # Play an exported video\n", program_name);
    printf("  %s -i input.jpg                      # Basic conversion\n", program_name);
    printf("  %s -i input.png -o output.txt -w 80  # Custom width output to file\n", program_name);
    printf("  %s -i input.jpg --mode braille --invert  # Braille with inverted colors\n", program_name);
    printf("  %s -i input.jpg --mode ansi --edge canny --canny-sigma 1.2 --canny-high 140 --canny-low 60\n", program_name);
}

int parse_numeric_arg(const char* arg, int min, int max, int* result) {
    if (!arg || !result) {
        return -1;
    }
    
    char* endptr;
    errno = 0;
    long val = strtol(arg, &endptr, 10);
    
    if (errno != 0 || *endptr != '\0' || val < min || val > max) {
        return -1;
    }
    
    *result = (int)val;
    return 0;
}

int parse_float_arg(const char* arg, float min, float max, float* result) {
    if (!arg || !result) {
        return -1;
    }
    
    char* endptr;
    errno = 0;
    float val = strtof(arg, &endptr);
    
    if (errno != 0 || *endptr != '\0' || val < min || val > max) {
        return -1;
    }
    
    *result = val;
    return 0;
}

int parse_arguments(int argc, char* argv[], AppConfig* config){
    if (!config || argc < 2) {
        return -1;
    }

    OutputModeSelection selected_output_mode = OUTPUT_MODE_UNSET;
    EdgeModeSelection selected_edge_mode = EDGE_MODE_UNSET;
    
    int opt;
    int option_index = 0;
    int width_set = 0;
    int height_set = 0;
    int legacy_output_flags_used = 0;
    int legacy_edge_flags_used = 0;
    int edge_threshold_set = 0;
    int canny_sigma_set = 0;
    int canny_high_set = 0;
    int canny_low_set = 0;
    int explicit_mode_set = 0;
    int explicit_edge_mode_set = 0;
    int explicit_threshold_set = 0;
    int explicit_dither_set = 0;
    int explicit_invert_set = 0;
    int explicit_threshold_value = 128;
    int explicit_dither_threshold = 128;
    int explicit_sobel_threshold = 128;
    float explicit_canny_sigma = 0.8f;
    int explicit_canny_high = 120;
    int explicit_canny_low = 50;
    OutputModeSelection explicit_output_mode = OUTPUT_MODE_UNSET;
    EdgeModeSelection explicit_edge_mode = EDGE_MODE_UNSET;
    const PresetDefinition* selected_preset = NULL;
    const char* short_options = "p:i:o:E:w:h:P:g:m:a:t:d:e:T:nr:qxLFWHbcBs::C::vV?";
    
    // Reset getopt state in case it was used elsewhere
    optind = 0;
    
    while ((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                config->play_cast_path = optarg;
                break;
                
            case 'i':
                config->input_path = optarg;
                break;
                
            case 'o':
                config->output_path = optarg;
                break;
                
            case 'E':
                config->export_cast_path = optarg;
                break;
                
            case 'q':
                config->no_terminal_output = 1;
                break;

            case 'F':
                if (isatty(STDOUT_FILENO)) {
                    struct winsize w;
                    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
                        config->width = w.ws_col;
                        config->height = w.ws_row > 1 ? w.ws_row - 1 : 0;
                        width_set = 1;
                        height_set = 1;
                    } else {
                        fprintf(stderr, "Warning: Could not determine terminal size\n");
                    }
                } else {
                    fprintf(stderr, "Warning: --fit-terminal used but output is not a terminal\n");
                }
                break;

            case 'W':
                if (isatty(STDOUT_FILENO)) {
                    struct winsize w;
                    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
                        config->width = w.ws_col;
                        width_set = 1;
                    } else {
                        fprintf(stderr, "Warning: Could not determine terminal size\n");
                    }
                } else {
                    fprintf(stderr, "Warning: --fit-terminal-w used but output is not a terminal\n");
                }
                break;

            case 'H':
                if (isatty(STDOUT_FILENO)) {
                    struct winsize w;
                    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
                        config->height = w.ws_row > 1 ? w.ws_row - 1 : 0;
                        height_set = 1;
                    } else {
                        fprintf(stderr, "Warning: Could not determine terminal size\n");
                    }
                } else {
                    fprintf(stderr, "Warning: --fit-terminal-h used but output is not a terminal\n");
                }
                break;

            case 'x':
                config->video = 1;
                break;

            case 'L':
                config->video_loop = 1;
                break;

            case 'P':
                {
                    const PresetDefinition* preset = find_preset(optarg);
                    if (!preset) {
                        fprintf(stderr, "Error: Unknown preset '%s'\n", optarg ? optarg : "(null)");
                        fprintf(stderr, "Available presets: ");
                        for (size_t i = 0; i < PRESET_COUNT; i++) {
                            fprintf(stderr, "%s%s", PRESETS[i].name, (i + 1 < PRESET_COUNT) ? ", " : "\n");
                        }
                        return -1;
                    }

                    selected_preset = preset;
                }
                break;
                
            case 'g':
                if (optarg && strlen(optarg) > 0) {
                    config->ramp.characters = optarg;
                    config->ramp.length = ascii_ramp_total_chars(optarg);
                } else {
                    fprintf(stderr, "Error: ASCII gradient cannot be empty\n");
                    return -1;
                }
                break;

            case 'm':
                if (!optarg) {
                    fprintf(stderr, "Error: --mode requires a value\n");
                    return -1;
                }

                if (strcmp(optarg, "ascii") == 0) {
                    selected_output_mode = OUTPUT_MODE_ASCII;
                } else if (strcmp(optarg, "braille") == 0) {
                    selected_output_mode = OUTPUT_MODE_BRAILLE;
                } else if (strcmp(optarg, "ansi") == 0) {
                    selected_output_mode = OUTPUT_MODE_ANSI;
                } else if (strcmp(optarg, "ansi-bg") == 0) {
                    selected_output_mode = OUTPUT_MODE_ANSI_BG;
                } else if (strcmp(optarg, "halfblock") == 0) {
                    selected_output_mode = OUTPUT_MODE_HALFBLOCK;
                } else {
                    fprintf(stderr, "Error: Invalid mode '%s'. Expected one of: ascii, braille, ansi, ansi-bg, halfblock\n", optarg);
                    return -1;
                }
                explicit_mode_set = 1;
                explicit_output_mode = selected_output_mode;
                break;
                
            case 'w':
                if (parse_numeric_arg(optarg, -1, 10000, &config->width) != 0) {
                    fprintf(stderr, "Error: Invalid width value '%s'\n", optarg);
                    return -1;
                }
                width_set = 1;
                break;
                
            case 'h':
                if (parse_numeric_arg(optarg, -1, 10000, &config->height) != 0) {
                    fprintf(stderr, "Error: Invalid height value '%s'\n", optarg);
                    return -1;
                }
                height_set = 1;
                break;
                
            case 'a':
                if (parse_numeric_arg(optarg, 0, 255, &config->alpha) != 0) {
                    fprintf(stderr, "Error: Invalid alpha value '%s'\n", optarg);
                    return -1;
                }
                break;
                
            case 'n':
                config->negative = 1;
                explicit_invert_set = 1;
                break;

            case 'c':
                legacy_output_flags_used = 1;
                config->color = 1;
                break;

            case 'B':
                legacy_output_flags_used = 1;
                config->color_background_mode = 1;
                break;
            
            case 'T':
                if (optarg && strlen(optarg) > 0) {
                    config->tiling_text = optarg;
                } else {
                    fprintf(stderr, "Error: Tiling text cannot be empty\n");
                    return -1;
                }
                break;

            case 't':
                config->threshold = 1;
                if (parse_numeric_arg(optarg, 0, 255, &config->threshold_value) != 0) {
                    fprintf(stderr, "Error: Invalid threshold value '%s'\n", optarg);
                    return -1;
                }
                explicit_threshold_set = 1;
                explicit_threshold_value = config->threshold_value;
                break;
                
            case 'd':
                config->dither = 1;
                if (parse_numeric_arg(optarg, 0, 255, &config->dither_threshold) != 0) {
                    fprintf(stderr, "Error: Invalid dither threshold value '%s'\n", optarg);
                    return -1;
                }
                explicit_dither_set = 1;
                explicit_dither_threshold = config->dither_threshold;
                break;

            case 'e':
                if (!optarg) {
                    fprintf(stderr, "Error: --edge requires a value\n");
                    return -1;
                }

                if (strcmp(optarg, "none") == 0) {
                    selected_edge_mode = EDGE_MODE_NONE;
                } else if (strcmp(optarg, "sobel") == 0) {
                    selected_edge_mode = EDGE_MODE_SOBEL;
                } else if (strcmp(optarg, "canny") == 0) {
                    selected_edge_mode = EDGE_MODE_CANNY;
                } else {
                    fprintf(stderr, "Error: Invalid edge mode '%s'. Expected one of: none, sobel, canny\n", optarg);
                    return -1;
                }
                explicit_edge_mode_set = 1;
                explicit_edge_mode = selected_edge_mode;
                break;
                
            case 's':
                legacy_edge_flags_used = 1;
                config->sobel_edge_detection = 1;
                if (optarg) {
                    if (parse_numeric_arg(optarg, 0, 255, &config->sobel_edge_detection_threshold) != 0) {
                        fprintf(stderr, "Error: Invalid Sobel threshold value '%s'\n", optarg);
                        return -1;
                    }
                }
                break;
                
            case 'C':
                legacy_edge_flags_used = 1;
                config->canny_edge_detection = 1;
                if (optarg) {
                    // Parse comma-separated Canny parameters
                    char* copy = strdup(optarg);
                    if (!copy) {
                        fprintf(stderr, "Error: Memory allocation failed\n");
                        return -1;
                    }
                    
                    char* token = strtok(copy, ",");
                    if (token) {
                        if (parse_float_arg(token, 0.1, 10.0, &config->canny_edge_detection_sigma) != 0) {
                            fprintf(stderr, "Error: Invalid Canny sigma value '%s'\n", token);
                            free(copy);
                            return -1;
                        }
                        
                        token = strtok(NULL, ",");
                        if (token) {
                            int high_threshold;
                            if (parse_numeric_arg(token, 0, 255, &high_threshold) != 0) {
                                fprintf(stderr, "Error: Invalid Canny high threshold value '%s'\n", token);
                                free(copy);
                                return -1;
                            }
                            config->canny_edge_detection_high_threshold = high_threshold;
                            
                            token = strtok(NULL, ",");
                            if (token) {
                                int low_threshold;
                                if (parse_numeric_arg(token, 0, 255, &low_threshold) != 0) {
                                    fprintf(stderr, "Error: Invalid Canny low threshold value '%s'\n", token);
                                    free(copy);
                                    return -1;
                                }
                                config->canny_edge_detection_low_threshold = low_threshold;
                            }
                        }
                    }
                    
                    free(copy);
                }
                break;
                
            case 'b':
                legacy_output_flags_used = 1;
                config->braille = 1;
                break;

            case 3:
                edge_threshold_set = 1;
                if (parse_numeric_arg(optarg, 0, 255, &config->sobel_edge_detection_threshold) != 0) {
                    fprintf(stderr, "Error: Invalid edge threshold value '%s'\n", optarg);
                    return -1;
                }
                explicit_sobel_threshold = config->sobel_edge_detection_threshold;
                break;

            case 4:
                canny_sigma_set = 1;
                if (parse_float_arg(optarg, 0.1, 10.0, &config->canny_edge_detection_sigma) != 0) {
                    fprintf(stderr, "Error: Invalid Canny sigma value '%s'\n", optarg);
                    return -1;
                }
                explicit_canny_sigma = config->canny_edge_detection_sigma;
                break;

            case 5:
                canny_high_set = 1;
                if (parse_numeric_arg(optarg, 0, 255, &config->canny_edge_detection_high_threshold) != 0) {
                    fprintf(stderr, "Error: Invalid Canny high threshold value '%s'\n", optarg);
                    return -1;
                }
                explicit_canny_high = config->canny_edge_detection_high_threshold;
                break;

            case 6:
                canny_low_set = 1;
                if (parse_numeric_arg(optarg, 0, 255, &config->canny_edge_detection_low_threshold) != 0) {
                    fprintf(stderr, "Error: Invalid Canny low threshold value '%s'\n", optarg);
                    return -1;
                }
                explicit_canny_low = config->canny_edge_detection_low_threshold;
                break;

            case 7:
                if (!optarg || strlen(optarg) == 0) {
                    fprintf(stderr, "Error: --debug-dir requires a non-empty directory path\n");
                    return -1;
                }
                config->debug_dir = optarg;
                break;

            case 8:
                if (parse_float_arg(optarg, 0.1f, 240.0f, &config->video_fps) != 0) {
                    fprintf(stderr, "Error: Invalid video FPS value '%s'\n", optarg);
                    return -1;
                }
                break;

            case 9:
                if (parse_numeric_arg(optarg, 1, 100000000, &config->video_max_frames) != 0) {
                    fprintf(stderr, "Error: Invalid video max frame count '%s'\n", optarg);
                    return -1;
                }
                break;
                
            case 'r':
                {
                    float ratio;
                    if (parse_float_arg(optarg, 0.01, 10.0, &ratio) != 0) {
                        fprintf(stderr, "Error: Invalid font aspect ratio '%s'\n", optarg);
                        return -1;
                    }
                    config->font_aspect_ratio = ratio;
                }
                break;
                
            case 'v':
                config->verbose = 1;
                break;
                
            case 'V':
                printf("printAscii v2.1.1\n");
                return 1; // Normal exit
                
            case '?':
                print_usage(argv[0]);
                return 1; // Normal exit
                
            default:
                fprintf(stderr, "Error: Unknown option\n");
                return -1;
        }
    }

    if (selected_preset) {
        apply_preset(selected_preset, config, &selected_output_mode, &selected_edge_mode);
    }

    if (explicit_mode_set) {
        selected_output_mode = explicit_output_mode;
    }

    if (explicit_edge_mode_set) {
        selected_edge_mode = explicit_edge_mode;
    }

    if (explicit_threshold_set) {
        config->threshold = 1;
        config->threshold_value = explicit_threshold_value;
    }

    if (explicit_dither_set) {
        config->dither = 1;
        config->dither_threshold = explicit_dither_threshold;
    }

    if (explicit_invert_set) {
        config->negative = 1;
    }

    if (edge_threshold_set) {
        config->sobel_edge_detection_threshold = explicit_sobel_threshold;
    }

    if (canny_sigma_set) {
        config->canny_edge_detection_sigma = explicit_canny_sigma;
    }

    if (canny_high_set) {
        config->canny_edge_detection_high_threshold = explicit_canny_high;
    }

    if (canny_low_set) {
        config->canny_edge_detection_low_threshold = explicit_canny_low;
    }

    if (!explicit_edge_mode_set && selected_preset && (edge_threshold_set || canny_sigma_set || canny_high_set || canny_low_set)) {
        selected_edge_mode = EDGE_MODE_UNSET;
    }

    if (selected_output_mode != OUTPUT_MODE_UNSET && legacy_output_flags_used) {
        fprintf(stderr, "Output mode conflict: use either --mode or legacy output flags (-b/-c/-B), not both\n");
        return -1;
    }

    if (selected_output_mode != OUTPUT_MODE_UNSET) {
        config->braille = 0;
        config->halfblock = 0;
        config->color = 0;
        config->color_background_mode = 0;

        if (selected_output_mode == OUTPUT_MODE_BRAILLE) {
            config->braille = 1;
        } else if (selected_output_mode == OUTPUT_MODE_ANSI) {
            config->color = 1;
        } else if (selected_output_mode == OUTPUT_MODE_ANSI_BG) {
            config->color = 1;
            config->color_background_mode = 1;
        } else if (selected_output_mode == OUTPUT_MODE_HALFBLOCK) {
            config->halfblock = 1;
            config->color = 1; // Needs color output for FG/BG combo
        }
    }

    if (selected_edge_mode != EDGE_MODE_UNSET && legacy_edge_flags_used) {
        fprintf(stderr, "Edge mode conflict: use either --edge or legacy edge flags (-s/-C), not both\n");
        return -1;
    }

    if (selected_edge_mode == EDGE_MODE_UNSET) {
        if (edge_threshold_set && (canny_sigma_set || canny_high_set || canny_low_set)) {
            fprintf(stderr, "Edge mode conflict: cannot infer edge mode when both Sobel and Canny parameters are provided; set --edge explicitly\n");
            return -1;
        }

        if (canny_sigma_set || canny_high_set || canny_low_set) {
            selected_edge_mode = EDGE_MODE_CANNY;
        } else if (edge_threshold_set) {
            selected_edge_mode = EDGE_MODE_SOBEL;
        }
    }

    if (selected_edge_mode == EDGE_MODE_NONE && (edge_threshold_set || canny_sigma_set || canny_high_set || canny_low_set)) {
        fprintf(stderr, "Edge mode conflict: --edge none cannot be combined with edge parameters\n");
        return -1;
    }

    if (selected_edge_mode == EDGE_MODE_SOBEL && (canny_sigma_set || canny_high_set || canny_low_set)) {
        fprintf(stderr, "Edge mode conflict: Sobel mode cannot be combined with Canny parameters\n");
        return -1;
    }

    if (selected_edge_mode == EDGE_MODE_CANNY && edge_threshold_set) {
        fprintf(stderr, "Edge mode conflict: Canny mode cannot be combined with --edge-threshold\n");
        return -1;
    }

    if (selected_edge_mode != EDGE_MODE_UNSET) {
        config->sobel_edge_detection = 0;
        config->canny_edge_detection = 0;

        if (selected_edge_mode == EDGE_MODE_SOBEL) {
            config->sobel_edge_detection = 1;
        } else if (selected_edge_mode == EDGE_MODE_CANNY) {
            config->canny_edge_detection = 1;
        }
    }
    
    // Handle aspect ratio calculations
    if (width_set && !height_set) {
        config->height = 0; // 0 means keep aspect ratio
    }
    if (!width_set && height_set) {
        config->width = 0; // 0 means keep aspect ratio
    }
    
    // Auto-adjust font aspect ratio for halfblock mode if not explicitly set
    if (config->halfblock && config->font_aspect_ratio == 0.45f) {
        config->font_aspect_ratio = 0.5f; // Halfblocks effectively double vertical resolution
    }

    return 0;
}

int validate_config(AppConfig* config) {
    if (config->play_cast_path) {
        return 0; // Skip other validations if we are just playing a cast
    }

    if (!config->input_path) {
        fprintf(stderr, "Input path is required\n");
        return -1;
    }

    if (config->video) {
        if (config->output_path) {
            fprintf(stderr, "Video mode currently supports terminal output only; remove --output\n");
            return -1;
        }

        if (config->no_terminal_output) {
            fprintf(stderr, "Video mode requires terminal output; remove --no-terminal-output\n");
            return -1;
        }
    } else {
        if (config->video_loop) {
            fprintf(stderr, "--video-loop can only be used with --video\n");
            return -1;
        }

        if (config->video_fps > 0.0f) {
            fprintf(stderr, "--video-fps can only be used with --video\n");
            return -1;
        }

        if (config->video_max_frames > 0) {
            fprintf(stderr, "--video-max-frames can only be used with --video\n");
            return -1;
        }
    }

    if (config->braille && config->color) {
        fprintf(stderr, "Output mode conflict: braille output cannot be combined with ANSI color output\n");
        return -1;
    }

    if (config->color_background_mode && !config->color) {
        fprintf(stderr, "Output mode conflict: ANSI background mode requires ANSI color mode\n");
        return -1;
    }

    if (config->sobel_edge_detection && config->canny_edge_detection) {
        fprintf(stderr, "Edge mode conflict: Sobel and Canny cannot be enabled at the same time\n");
        return -1;
    }

    if ((config->width < -1) || (config->height < -1) || (config->width == 0 && config->height == 0)) {
        fprintf(stderr, "Invalid output dimensions: %dx%d\n", config->width, config->height);
        return -1;
    }

    if (config->alpha < 0 || config->alpha > 255) {
        fprintf(stderr, "Alpha value must be between 0 and 255\n");
        return -1;
    }

    if (config->dither_threshold < 0 || config->dither_threshold > 255) {
        fprintf(stderr, "Threshold value must be between 0 and 255\n");
        return -1;
    }

    if (config->sobel_edge_detection_threshold < 0 || config->sobel_edge_detection_threshold > 255) {
        fprintf(stderr, "Threshold value must be between 0 and 255\n");
        return -1;
    }

    if (config->canny_edge_detection_high_threshold < 0 || config->canny_edge_detection_high_threshold > 255) {
        fprintf(stderr, "Threshold value must be between 0 and 255\n");
        return -1;
    }

    if (config->canny_edge_detection_low_threshold < 0 || config->canny_edge_detection_low_threshold > 255) {
        fprintf(stderr, "Threshold value must be between 0 and 255\n");
        return -1;
    }

    if(config->font_aspect_ratio < 0){
        fprintf(stderr, "Font aspect ratio must be positive\n");
        return -1;
    }

    if (config->threshold_value < 0 || config->threshold_value > 255) {
        fprintf(stderr, "Threshold value must be between 0 and 255\n");
        return -1;
    }

    if (!config->debug_dir || config->debug_dir[0] == '\0') {
        fprintf(stderr, "Debug directory path must not be empty\n");
        return -1;
    }

    return 0;
}

void print_config(const AppConfig* config) {
    printf("Configuration:\n");
    printf("  Input file: %s\n", config->input_path);
    printf("  Output file: %s\n", config->output_path ? config->output_path : "(terminal only)");
    printf("  Video mode: %s\n", config->video ? "Enabled" : "Disabled");
    if (config->video) {
        printf("  Video loop: %s\n", config->video_loop ? "Enabled" : "Disabled");
    }
    if (config->video_fps > 0.0f) {
        printf("  Video FPS override: %.2f\n", config->video_fps);
    }
    if (config->video_max_frames > 0) {
        printf("  Video max frames: %d\n", config->video_max_frames);
    }
    printf("  Debug directory: %s\n", config->debug_dir ? config->debug_dir : ".");
    printf("  Dimensions: %d x %d characters\n", config->width, config->height);
    printf("  ASCII gradient: \"%s\"\n", config->ramp.characters);
    
    if (config->braille)
        printf("  Output mode: Braille\n");
    else
        printf("  Output mode: ASCII\n");
    
    if (config->negative)
        printf("  Color mode: Inverted\n");
    
    if (config->dither)
        printf("  Dithering: Enabled (threshold: %d)\n", config->dither_threshold);
    
    if (config->canny_edge_detection)
        printf("  Edge detection: Canny (sigma: %.1f, high: %d, low: %d)\n", 
               config->canny_edge_detection_sigma,
               config->canny_edge_detection_high_threshold,
               config->canny_edge_detection_low_threshold);
    else if (config->sobel_edge_detection)
        printf("  Edge detection: Sobel (threshold: %d)\n", 
               config->sobel_edge_detection_threshold);
    
    printf("\n");
}