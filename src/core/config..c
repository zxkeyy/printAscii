#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include "core/config.h"

const AsciiRamp DEFAULT_RAMP = {
    .characters = " .:-=+*#@&8B$@",
    .length = 14
};

const AppConfig DEFAULT_CONFIG = {
    .input_path = NULL,
    .output_path = NULL,
    .no_terminal_output = 0,
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
    .font_aspect_ratio = 0.45,
    .verbose = 0,
    .ramp = {0} // Initialize all to zero and set it later in get function, because C ¯\_(ツ)_/¯.
};

AppConfig get_default_config() {
    AppConfig config = DEFAULT_CONFIG;
    // Copy the default ramp
    config.ramp = DEFAULT_RAMP;

    return config;
}


struct option long_options[] = {
    {"input", required_argument, NULL, 'i'},
    {"output", required_argument, NULL, 'o'},
    {"no-terminal-output", no_argument, NULL, 'q'},
    {"ascii-gradient", required_argument, NULL, 'g'},
    {"gradient-preset", required_argument, NULL, 'G'},
    {"width", required_argument, NULL, 'w'},
    {"height", required_argument, NULL, 'h'},
    {"alpha", required_argument, NULL, 'a'},
    {"threshold", optional_argument, NULL, 't'},
    {"negative", no_argument, NULL, 'n'},
    {"dither", optional_argument, NULL, 'd'},
    {"sobel-edge-detection", optional_argument, NULL, 's'},
    {"canny-edge-detection", optional_argument, NULL, 'C'},
    {"braille", no_argument, NULL, 'b'},
    {"font-aspect-ratio", required_argument, NULL, 'r'},
    {"color", no_argument, NULL, 'c'},
    {"color-background", no_argument, NULL, 'B'},
    {"tiling-text", required_argument, NULL, 'T'},
    {"preview", no_argument, NULL, 'p'},
    {"verbose", no_argument, NULL, 'v'},
    {"version", no_argument, NULL, 'V'},
    {"config", required_argument, NULL, 1},
    {"save-config", required_argument, NULL, 2},
    {"help", no_argument, NULL, '?'},
    {NULL, 0, NULL, 0}
};

void print_usage(const char* program_name) {
    printf("Usage: %s -i <input> [options]\n\n", program_name);
    printf("Image to ASCII/ANSI/Unicode Art Converter\n\n");
    
    printf("Input/Output Options:\n");
    printf("  -i, --input <file>         Input image file (JPG, PNG, TGA, BMP, etc.)\n");
    printf("  -o, --output <file>        Output file (default: print to terminal)\n");
    printf("  -q, --no-terminal-output   Don't print the result to terminal\n\n");
    
    printf("Display Options:\n");
    printf("  -g, --ascii-gradient <str> ASCII gradient (default: ' .:-=+*#@&8B$@')\n");
    printf("  -w, --width <n>            Output width in characters (default: 100)\n");
    printf("                             if only height is specified, width will be calculated to keep image aspect ratio\n");
    printf("                             Use -1 to keep source image width\n");
    printf("  -h, --height <n>           Output height in characters\n");
    printf("                             if only width is specified, height will be calculated to keep image aspect ratio\n");
    printf("                             Use -1 to keep source image height\n");
    printf("  -c, --color                Use ANSI colors for output\n");
    printf("  -B, --color-background     Use ANSI colors for background color output\n");
    printf("  -T, --tiling_text <str>    A string that will be used to tile the output if color is used (default: '0')\n");
    printf("  -b, --braille              Convert image to braille patterns\n");
    printf("  -r, --font-aspect-ratio <f> Font width to height ratio (default: 0.45)\n");
    printf("                             Change this if the output aspect ratio is incorrect\n\n");
    
    printf("Processing Options:\n");
    printf("  -a, --alpha <0-255>        Background brightness for transparency (default: 0)\n");
    printf("  -t, --threshold <0-255>    Threshold for black/white output (default: 128)\n");
    printf("  -n, --negative             Invert colors\n");
    printf("  -d, --dither <0-255>       Apply Floyd-Steinberg dithering (default: 128)\n\n");
    
    printf("Edge Detection:\n");
    printf("  -s, --sobel <0-255>        Sobel edge detection threshold (default: 128)\n");
    printf("  -C, --canny <s,h,l>        Canny edge detection with parameters:\n");
    printf("                             s=sigma (default: 0.8)\n");
    printf("                             h=high threshold (default: 120)\n");
    printf("                             l=low threshold (default: 50)\n\n");
    
    printf("General Options:\n");
    printf("  -v, --verbose              Display processing information\n");
    printf("  -V, --version              Show version information\n");
    printf("  --help                     Display this help message\n\n");
    
    printf("Examples:\n");
    printf("  %s -i input.jpg                      # Basic conversion\n", program_name);
    printf("  %s -i input.png -o output.txt -w 80  # Custom width output to file\n", program_name);
    printf("  %s -i input.jpg -b -n                # Braille with inverted colors\n", program_name);
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
    
    int opt;
    int option_index = 0;
    int width_set = 0;
    int height_set = 0;
    const char* short_options = "i:o:w:h:g:G:a:t::d::s::C::BT:nbcpr:vV?";
    
    // Reset getopt state in case it was used elsewhere
    optind = 0;
    
    while ((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        switch (opt) {
            case 'i':
                config->input_path = optarg;
                break;
                
            case 'o':
                config->output_path = optarg;
                break;
                
            case 'q':
                config->no_terminal_output = 1;
                break;
                
            case 'g':
                if (optarg && strlen(optarg) > 0) {
                    config->ramp.characters = optarg;
                    config->ramp.length = strlen(optarg);
                } else {
                    fprintf(stderr, "Error: ASCII gradient cannot be empty\n");
                    return -1;
                }
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
                break;

            case 'c':
                config->color = 1;
                break;

            case 'B':
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
                if (optarg) {
                    if (parse_numeric_arg(optarg, 0, 255, &config->threshold_value) != 0) {
                        fprintf(stderr, "Error: Invalid threshold value '%s'\n", optarg);
                        return -1;
                    }
                }
                break;
                
            case 'd':
                config->dither = 1;
                if (optarg) {
                    if (parse_numeric_arg(optarg, 0, 255, &config->dither_threshold) != 0) {
                        fprintf(stderr, "Error: Invalid dither threshold value '%s'\n", optarg);
                        return -1;
                    }
                }
                break;
                
            case 's':
                config->sobel_edge_detection = 1;
                if (optarg) {
                    if (parse_numeric_arg(optarg, 0, 255, &config->sobel_edge_detection_threshold) != 0) {
                        fprintf(stderr, "Error: Invalid Sobel threshold value '%s'\n", optarg);
                        return -1;
                    }
                }
                break;
                
            case 'C':
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
                config->braille = 1;
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
                printf("printAscii v2.0.4\n");
                return 1; // Normal exit
                
            case '?':
                print_usage(argv[0]);
                return 1; // Normal exit
                
            default:
                fprintf(stderr, "Error: Unknown option\n");
                return -1;
        }
    }
    
    // Handle aspect ratio calculations
    if (width_set && !height_set) {
        config->height = 0; // 0 means keep aspect ratio
    }
    if (!width_set && height_set) {
        config->width = 0; // 0 means keep aspect ratio
    }
    
    return 0;
}

int validate_config(AppConfig* config) {
    if (!config->input_path) {
        fprintf(stderr, "Input path is required\n");
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

    return 0;
}

void print_config(const AppConfig* config) {
    printf("Configuration:\n");
    printf("  Input file: %s\n", config->input_path);
    printf("  Output file: %s\n", config->output_path ? config->output_path : "(terminal only)");
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