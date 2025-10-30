#ifndef IMAGE_PIPELINE_H
#define IMAGE_PIPELINE_H

#include "core/image.h"
#include "core/config.h"
#include <stddef.h>

typedef enum {
    PIPELINE_SUCCESS = 0,
    PIPELINE_ERROR_MEMORY,
    PIPELINE_ERROR_PROCESSING,
    PIPELINE_ERROR_CONVERSION,
    PIPELINE_ERROR_OUTPUT
} PipelineStatus;

typedef struct {
    char* output_data;
    size_t output_length;
    PipelineStatus status;
    char* error_message;
} ProcessingResult;

// Main pipeline function - processes an image according to config
ProcessingResult* image_pipeline_process(Image* img, const AppConfig* config);

// Helper functions for pipeline stages
PipelineStatus pipeline_apply_preprocessing(Image* img, const AppConfig* config);
char* pipeline_convert_to_output(const Image* img, const AppConfig* config);
PipelineStatus pipeline_handle_output(const ProcessingResult* result, const AppConfig* config);

// Utility functions
void pipeline_calculate_dimensions(AppConfig* config, const Image* img);
ProcessingResult* pipeline_create_result(void);
void pipeline_free_result(ProcessingResult* result);

#endif // IMAGE_PIPELINE_H