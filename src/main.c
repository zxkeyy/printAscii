#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "core/image.h"
#include "core/config.h"
#include "core/image_pipeline.h"
#include "core/video_pipeline.h"
#include "io/image_loader.h"
#include "io/image_saver.h"
#include "utilities/debug_artifacts.h"
#include "utilities/print_utf16_string.h"

int main(int argc, char *argv[]) {
    AppConfig config = get_default_config();

    int parse_status = parse_arguments(argc, argv, &config);
    if (parse_status < 0) {
        return EXIT_FAILURE;
    }
    if (parse_status > 0) {
        return EXIT_SUCCESS;
    }

    if (validate_config(&config) != 0) {
        return EXIT_FAILURE;
    }

    if (config.video) {
        return video_pipeline_run_terminal(&config);
    }

    Image* img = image_load_from_file(config.input_path);
    if (!img) {
        fprintf(stderr, "Failed to load image\n");
        return EXIT_FAILURE;
    }

    // Debug: Save original image if verbose mode
    if (config.verbose) {
        save_debug_image(img, &config, "1original.png");
    }

    // Process the image through the pipeline
    ProcessingResult* result = image_pipeline_process(img, &config);
    
    // Handle pipeline errors
    if (!result) {
        fprintf(stderr, "Failed to create processing result\n");
        image_free(img);
        return EXIT_FAILURE;
    }
    
    if (result->status != PIPELINE_SUCCESS) {
        fprintf(stderr, "Pipeline failed: %s\n", 
                result->error_message ? result->error_message : "Unknown error");
        pipeline_free_result(result);
        image_free(img);
        return EXIT_FAILURE;
    }

    // Clean up
    pipeline_free_result(result);
    image_free(img);
    return EXIT_SUCCESS;
}
