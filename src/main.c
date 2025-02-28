#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "core/image.h"
#include "core/config.h"
#include "preprocessing/dither.h"
#include "preprocessing/grayscale.h"
#include "preprocessing/invert.h"
#include "preprocessing/resize.h"
#include "preprocessing/sobel_edge_detection.h"
#include "preprocessing/canny_edge_detection.h"
#include "io/image_loader.h"
#include "io/image_saver.h"
#include "core/ascii_ramp.h"
#include "conversion/intensity_map.h"
#include "conversion/image_to_braille.h"
#include "conversion/image_to_ansi.h"
#include "utilities/print_utf16_string.h"


int main(int argc, char *argv[]) {
    AppConfig config = get_default_config();

    if (parse_arguments(argc, argv, &config) != 0) {
        //print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (validate_config(&config) != 0) {
        return EXIT_FAILURE;
    }

    Image* img = image_load_from_file(config.input_path);
    if (!img) {
        fprintf(stderr, "Failed to load image\n");
        return EXIT_FAILURE;
    }
    //debug
    image_save_to_png_file(img, "1original.png");

    if (config.width == -1){
        config.width = img->width;
    }
    if (config.height == -1){
        config.height = (int)(float)img->height * config.font_aspect_ratio;
    }

    // calculate width or height if one of them is 0 to keep aspect ratio
    if (config.width == 0){
        config.width = (int)(((float)config.height / img->height * img->width) / config.font_aspect_ratio);
    }
    if (config.height == 0){
        config.height = (int)(((float)config.width / img->width * img->height) * config.font_aspect_ratio);
    }

    image_resize(img, config.width, config.height);
    //debug
    image_save_to_png_file(img, "2resized.png");


    //image_to_grayscale(img, config.alpha);
    //debug
    image_save_to_png_file(img, "3gray.png");

    if(config.canny_edge_detection){
        canny_edge_detection(img, config.canny_edge_detection_sigma, config.canny_edge_detection_high_threshold, config.canny_edge_detection_low_threshold);
        //debug
        image_save_to_png_file(img, "4cannyedgedetect.png");
    }

    if (config.sobel_edge_detection){
        sobel_edge_detection(img, config.sobel_edge_detection_threshold);
        //debug
        image_save_to_png_file(img, "4sobeledgedetect.png");
    }

    if (config.negative) {
        invert_image(img);
        //debug
        image_save_to_png_file(img, "5inverted.png");
    }

    if (config.dither) {
        floyd_steinberg_dither(img, config.dither_threshold);
        //debug
        image_save_to_png_file(img, "6dithered.png");
    }

    if (config.braille) {
        int16_t* output = image_to_braille(img, config.threshold_value);
        if (!config.no_terminal_output) {
            print_utf16_string(output);
        }

        if (config.output_path) {
            FILE* file = fopen(config.output_path, "w");
            if (!file) {
                perror("Failed to open output file");
                free(output);
                image_free(img);
                return EXIT_FAILURE;
            }

            print_utf16_string_to_file(output, config.output_path);
            fclose(file);
        }

        free(output);
        image_free(img);
        return EXIT_SUCCESS;
    } else {
        //char* output = intensity_map(img, &config.ramp);
        char* output = image_to_ansi(img, " ", (RGBColor){config.alpha, config.alpha, config.alpha});
        //char* output = RGBA_image_to_ansi(img, " ", config.alpha);
        if (!output) {
            fprintf(stderr, "Failed to generate intensity map\n");
            image_free(img);
            return EXIT_FAILURE;
        }

        if (!config.no_terminal_output) {
            printf("%s", output);
        }

        if (config.output_path) {
            FILE* file = fopen(config.output_path, "w");
            if (!file) {
                perror("Failed to open output file");
                free(output);
                image_free(img);
                return EXIT_FAILURE;
            }

            fprintf(file, "%s", output);
            fclose(file);
        }

        free(output);
        image_free(img);
        return EXIT_SUCCESS;
    }
}
