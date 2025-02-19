#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "core/config.h"

struct option long_options[] = {
    {"input", required_argument, NULL, 'i'},
    {"output", required_argument, NULL, 'o'},
    {"no-terminal-output", no_argument, NULL, 'q'},
    {"ascii-gradient", required_argument, NULL, 'g'},   
    {"width", required_argument, NULL, 'w'},
    {"height", required_argument, NULL, 'h'},
    {"alpha", required_argument, NULL, 'a'},
    {"threshold", required_argument, NULL, 't'},
    {"negative", no_argument, NULL, 'n'},
    {"dither", optional_argument, NULL, 'd'},
    {"sobel-edge-detection", optional_argument, NULL, 's'},
    {"font-aspect-ratio", required_argument, NULL, 0},
    {"verbose", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, '?'},
    {NULL, 0, NULL, 0}
};

void print_usage(const char* program_name) {
    printf("Usage: %s [options]\n"
           "Options:\n"
           "  -i, --input    <input>     Input file (JPG, PNG, TGA, BMP, PSD, GIF, HDR, PIC image)\n"
           "  -o, --output   <output>    Output file (optional)\n"
           "  -q, --no-terminal-output   Quiet mode disables terminal output\n"
           "  -g --ascii-gradient <gradient> ASCII gradient to use (default: ' .:-=+*#@&8B$@')\n"
           "  -w, --width    <width>     Output width in characters ('-1' to keep the width of the source image, default: 100 character)\n"
           "  -h, --height   <height>    Output height in characters ('-1' to keep the height of the source image)\n"
           "  -a, --alpha    <alpha>     Defines brightness of background for images with alpha transparency (0 - 255, default=0)\n"
           "  -n, --negative             Invert colors\n"
           "  -d  --dither               Apply floyd steinberg dithering to the image, (optional threshold argument 0 - 255, default=128)\n"
           "  -s  --sobel-edge-detection    <threshold> Apply sobel edge detection, (optional threshold argument 0 - 255, default=128)\n"
           "  --font-aspect-ratio <ratio> Width to height ratio of the font (default: 0.45)\n"
           "  -v, --verbose              Verbose output\n"
           "  -?, --help                 Display this help message\n",
           program_name);
}

int parse_arguments(int argc, char* argv[], AppConfig* config){
    int opt;
    int optind = 0;
    //To calculate if user wants to keep aspect ratio
    int width_set = 0;
    int height_set = 0;
    while ((opt = getopt_long(argc, argv, "i:o:w:h:g:a:t:s::nqrdv?", long_options, &optind)) != -1) {
        switch (opt) {
            case 'i': config->input_path = optarg; break;
            case 'o': config->output_path = optarg; break;
            case 'q': config->no_terminal_output = 1; break;
            case 'g': config->ramp.characters = optarg; config->ramp.length = strlen(optarg); break;
            case 'w': config->width = atoi(optarg); width_set = 1; break;
            case 'h': config->height = atoi(optarg); height_set = 1; break;
            case 'a': config->alpha = atoi(optarg); break;
            case 'n': config->negative = 1; break;
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

    if(config->font_aspect_ratio < 0){
        fprintf(stderr, "Font aspect ratio must be positive\n");
        return -1;
    }

    return 0;
}