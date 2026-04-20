#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utilities/debug_artifacts.h"
#include "io/image_saver.h"

void save_debug_image(const Image* img, const AppConfig* config, const char* filename) {
    if (!img || !config || !filename || filename[0] == '\0') {
        return;
    }

    const char* debug_dir = (config->debug_dir && config->debug_dir[0] != '\0') ? config->debug_dir : ".";
    size_t dir_len = strlen(debug_dir);
    size_t filename_len = strlen(filename);
    int add_separator = (dir_len > 0 && debug_dir[dir_len - 1] != '/');
    size_t path_len = dir_len + (size_t)add_separator + filename_len + 1;

    char* output_path = malloc(path_len);
    if (!output_path) {
        fprintf(stderr, "Warning: Failed to allocate debug path for '%s'\n", filename);
        return;
    }

    if (add_separator) {
        snprintf(output_path, path_len, "%s/%s", debug_dir, filename);
    } else {
        snprintf(output_path, path_len, "%s%s", debug_dir, filename);
    }

    if (!image_save_to_png_file(img, output_path)) {
        fprintf(stderr, "Warning: Failed to save debug image '%s'\n", output_path);
    }

    free(output_path);
}