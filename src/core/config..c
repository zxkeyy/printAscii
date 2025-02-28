#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
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
    .ramp = {0} // Initialize all to zero because C. ¯\_(ツ)_/¯
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
    {"width", required_argument, NULL, 'w'},
    {"height", required_argument, NULL, 'h'},
    {"alpha", required_argument, NULL, 'a'},
    {"threshold", optional_argument, NULL, 't'},
    {"negative", no_argument, NULL, 'n'},
    {"dither", optional_argument, NULL, 'd'},
    {"sobel-edge-detection", optional_argument, NULL, 's'},
    {"canny-edge-detection", optional_argument, NULL, 'c'},
    {"braille", no_argument, NULL, 'b'},
    {"font-aspect-ratio", required_argument, NULL, 0},
    {"verbose", no_argument, NULL, 'v'},
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
    printf("  -b, --braille              Convert image to braille patterns\n");
    printf("  -r, --font-aspect-ratio <n> Font width to height ratio (default: 0.45)\n\n");
    printf("                             Change this if the output aspect ratio is incorrect\n");
    
    printf("Processing Options:\n");
    printf("  -a, --alpha <0-255>        Background brightness for transparency (default: 0)\n");
    printf("  -t, --threshold <0-255>    Threshold for black/white output (default: 128)\n");
    printf("  -n, --negative             Invert colors\n");
    printf("  -d, --dither <0-255>       Apply Floyd-Steinberg dithering (default: 128)\n\n");
    
    printf("Edge Detection:\n");
    printf("  -s, --sobel <0-255>        Sobel edge detection threshold (default: 128)\n");
    printf("  -c, --canny <s,h,l>        Canny edge detection with parameters:\n");
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

int parse_arguments(int argc, char* argv[], AppConfig* config){
    int opt;
    int optind = 0;
    //To calculate if user wants to keep aspect ratio
    int width_set = 0;
    int height_set = 0;
    while ((opt = getopt_long(argc, argv, "i:o:w:h:g:a:t::d::s::c::nbqrv?", long_options, &optind)) != -1) {
        switch (opt) {
            case 'i': config->input_path = optarg; break;
            case 'o': config->output_path = optarg; break;
            case 'q': config->no_terminal_output = 1; break;
            case 'g': config->ramp.characters = optarg; config->ramp.length = strlen(optarg); break;
            case 'w': config->width = atoi(optarg); width_set = 1; break;
            case 'h': config->height = atoi(optarg); height_set = 1; break;
            case 'a': config->alpha = atoi(optarg); break;
            case 'n': config->negative = 1; break;
            case 't': 
                config->threshold = 1; 
                if (optarg)
                    config->threshold_value = atoi(optarg);
                break;
            case 'd': 
                config->dither = 1; 
                if (optarg)
                    config->dither_threshold = atoi(optarg);
                break;
            case 's': 
                config->sobel_edge_detection = 1; 
                if (optarg)
                    config->sobel_edge_detection_threshold = atoi(optarg);
                break;
            case 'c':
                config->canny_edge_detection = 1;
                if(optarg){
                    int params = 0;
                    char *temp = optarg;
                    while (*temp)
                    {
                        if(*temp == ',') params++;
                        temp++;
                    }
                    params++;
                    if (params >= 3) {
                        sscanf(optarg, "%f,%d,%d", &config->canny_edge_detection_sigma, &config->canny_edge_detection_high_threshold, &config->canny_edge_detection_low_threshold);
                    } else if (params == 2) {
                        sscanf(optarg, "%f,%d", &config->canny_edge_detection_sigma, &config->canny_edge_detection_high_threshold);
                    } else if (params == 1) {
                        sscanf(optarg, "%f", &config->canny_edge_detection_sigma);
                    } 
                }
                break;
            case 'b': config->braille = 1; break;
            case 'v': config->verbose = 1; break;
            case '?': print_usage(argv[0]); return 1;
            case 0: 
                if (strcmp(long_options[optind].name, "font-aspect-ratio") == 0) {
                    config->font_aspect_ratio = atof(optarg);
                }
                break;
            default:
                return -1;
        }
    }

    if(width_set && !height_set){
        config->height = 0; // 0 means keep aspect ratio
    }
    if(!width_set && height_set){
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