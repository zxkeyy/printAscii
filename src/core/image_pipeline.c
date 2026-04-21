#include "core/image_pipeline.h"
#include "preprocessing/resize.h"
#include "preprocessing/grayscale.h"
#include "preprocessing/canny_edge_detection.h"
#include "preprocessing/sobel_edge_detection.h"
#include "preprocessing/invert.h"
#include "preprocessing/dither.h"
#include "conversion/image_to_ansi.h"
#include "conversion/image_to_braille.h"
#include "conversion/intensity_map.h"
#include "io/image_saver.h"
#include "utilities/debug_artifacts.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Main pipeline function
ProcessingResult* image_pipeline_process(Image* img, const AppConfig* config) {
    ProcessingResult* result = pipeline_create_result();
    if (!result) {
        return NULL;
    }
    
    // Stage 1: Calculate and apply dimensions
    AppConfig mutable_config = *config; // Create mutable copy
    pipeline_calculate_dimensions(&mutable_config, img);
    
    // Stage 2: Apply preprocessing pipeline
    const char* preprocessing_stage = NULL;
    result->status = pipeline_apply_preprocessing(img, &mutable_config, &preprocessing_stage);
    if (result->status != PIPELINE_SUCCESS) {
        const char* stage = preprocessing_stage ? preprocessing_stage : "unknown";
        char message[128];
        snprintf(message, sizeof(message), "Preprocessing failed at stage: %s", stage);
        result->error_message = strdup(message);
        return result;
    }
    
    // Stage 3: Convert to output format
    result->output_data = pipeline_convert_to_output(img, &mutable_config);
    if (!result->output_data) {
        result->status = PIPELINE_ERROR_CONVERSION;
        result->error_message = strdup("Output conversion failed");
        return result;
    }
    
    result->output_length = strlen(result->output_data);
    result->status = PIPELINE_SUCCESS;
    
    // Stage 4: Handle output (terminal/file)
    PipelineStatus output_status = pipeline_handle_output(result, &mutable_config);
    if (output_status != PIPELINE_SUCCESS) {
        result->status = output_status;
        // Keep the output_data for caller, just mark the output handling failure
    }
    
    return result;
}

PipelineStatus pipeline_apply_preprocessing(Image* img, const AppConfig* config, const char** error_stage) {
    if (error_stage) {
        *error_stage = NULL;
    }

    // Step 1: Resize
    if (config->width > 0 && config->height > 0) {
        if (!image_resize(img, config->width, config->height)) {
            if (error_stage) {
                *error_stage = "resize";
            }
            return PIPELINE_ERROR_PROCESSING;
        }
        // Debug output
        if (config->verbose) {
            save_debug_image(img, config, "2resized.png");
        }
    }
    
    // Step 2: Grayscale conversion
    if (!config->color) {
        if (!image_to_grayscale(img, config->alpha)) {
            if (error_stage) {
                *error_stage = "grayscale";
            }
            return PIPELINE_ERROR_PROCESSING;
        }
        // Debug output
        if (config->verbose) {
            save_debug_image(img, config, "3gray.png");
        }
    }
    
    // Step 3: Edge detection
    if (config->canny_edge_detection) {
        if (!canny_edge_detection(img, config->canny_edge_detection_sigma,
                                  config->canny_edge_detection_high_threshold,
                                  config->canny_edge_detection_low_threshold)) {
            if (error_stage) {
                *error_stage = "canny-edge-detection";
            }
            return PIPELINE_ERROR_PROCESSING;
        }
        // Debug output
        if (config->verbose) {
            save_debug_image(img, config, "4cannyedgedetect.png");
        }
    }
    
    if (config->sobel_edge_detection) {
        if (!sobel_edge_detection(img, config->sobel_edge_detection_threshold)) {
            if (error_stage) {
                *error_stage = "sobel-edge-detection";
            }
            return PIPELINE_ERROR_PROCESSING;
        }
        // Debug output
        if (config->verbose) {
            save_debug_image(img, config, "4sobeledgedetect.png");
        }
    }
    
    // Step 4: Inversion
    if (config->negative) {
        if (!invert_image(img)) {
            if (error_stage) {
                *error_stage = "invert";
            }
            return PIPELINE_ERROR_PROCESSING;
        }
        // Debug output
        if (config->verbose) {
            save_debug_image(img, config, "5inverted.png");
        }
    }
    
    // Step 5: Dithering
    if (config->dither) {
        if (!floyd_steinberg_dither(img, config->dither_threshold)) {
            if (error_stage) {
                *error_stage = "dither";
            }
            return PIPELINE_ERROR_PROCESSING;
        }
        // Debug output
        if (config->verbose) {
            save_debug_image(img, config, "6dithered.png");
        }
    }
    
    return PIPELINE_SUCCESS;
}

char* pipeline_convert_to_output(const Image* img, const AppConfig* config) {
    // Cast away const since your conversion functions don't expect const
    Image* mutable_img = (Image*)img;
    
    if (config->color) {
        return image_to_ansi(mutable_img, config->tiling_text, 
                           (RGBColor){config->alpha, config->alpha, config->alpha}, 
                           config->color_background_mode);
    } else if (config->braille) {
        return image_to_braille(mutable_img, config->threshold_value);
    } else {
        return intensity_map(mutable_img, &config->ramp);
    }
}

PipelineStatus pipeline_handle_output(const ProcessingResult* result, const AppConfig* config) {
    // Terminal output
    if (!config->no_terminal_output) {
        printf("%s", result->output_data);
    }
    
    // File output
    if (config->output_path) {
        FILE* file = fopen(config->output_path, "w");
        if (!file) {
            perror("Failed to open output file");
            return PIPELINE_ERROR_OUTPUT;
        }
        
        if (fprintf(file, "%s", result->output_data) < 0) {
            fclose(file);
            return PIPELINE_ERROR_OUTPUT;
        }
        
        fclose(file);
    }
    
    return PIPELINE_SUCCESS;
}

void pipeline_calculate_dimensions(AppConfig* config, const Image* img) {
    // Handle default dimensions
    if (config->width == -1) {
        config->width = img->width;
    }
    if (config->height == -1) {
        config->height = (int)(img->height * config->font_aspect_ratio);
    }
    
    // Calculate width or height if one is 0 to keep aspect ratio
    if (config->width == 0) {
        config->width = (int)(((float)config->height / img->height * img->width) / config->font_aspect_ratio);
    }
    if (config->height == 0) {
        config->height = (int)(((float)config->width / img->width * img->height) * config->font_aspect_ratio);
    }
}

ProcessingResult* pipeline_create_result(void) {
    ProcessingResult* result = malloc(sizeof(ProcessingResult));
    if (!result) {
        return NULL;
    }
    
    result->output_data = NULL;
    result->output_length = 0;
    result->status = PIPELINE_ERROR_MEMORY;
    result->error_message = NULL;
    
    return result;
}

void pipeline_free_result(ProcessingResult* result) {
    if (result) {
        free(result->output_data);
        free(result->error_message);
        free(result);
    }
}