#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "conversion/image_to_ansi.h"
#include "core/ascii_ramp.h"

char* image_to_ansi(Image* img, char* tiling_string, RGBColor background_color, int background){
    if (img == NULL) {
        fprintf(stderr, "Image is NULL\n");
        return NULL;
    }

    // ANSI escape code for colored character, uses 24-bit color, 38 for character color and 48 for background color
    const char* format_string = background ? "\033[48;2;%03d;%03d;%03dm%c" : "\033[38;2;%03d;%03d;%03dm%c";

    const int tiling_string_length = strlen(tiling_string);
    int index = 0;

    // Calculate safe buffer size
    // 25 characters per pixel (including escape codes), 6 characters for newline and reset, 10 characters for final reset and null terminator
    size_t buffer_size = (img->width * img->height * 25) + (img->height * 6) + 5;
    char* output = malloc(buffer_size);
    if (!output) {
        perror("Failed to allocate output buffer");
        return NULL;
    }

    size_t pos = 0; // To track osition in output buffer

    for (int y = 0; y < img->height; y++) {
        for(int x = 0; x < img->width; x++) {
            const RGBColor color = get_rgb_color(img, x, y, &background_color);
            
            // Write color and character to output buffer
            int written = snprintf(&output[pos], buffer_size - pos, format_string, color.r, color.g, color.b, tiling_string[index++ % tiling_string_length]);
    
            // Check for buffer overflow
            if (written < 0 || written >= buffer_size - pos) {
                fprintf(stderr, "Buffer overflow when writing to output\n");
                free(output);
                return NULL;
            }

            pos += written;
        }
        int written = snprintf(output + pos, buffer_size - pos, "\033[0m\n"); // Reset color and newline
        pos += written;
    }
    snprintf(output + pos, buffer_size - pos, "\033[0m");

    return output;
}

char* image_to_alpha_ansi(Image* img, char* tiling_string, AsciiRamp ramp, int background){
    if (img == NULL) {
        fprintf(stderr, "Image is NULL\n");
        return NULL;
    }

    // ANSI escape code for colored character, uses 24-bit color, 38 for character color and 48 for background color
    const char* format_string = background ? "\033[48;2;%03d;%03d;%03dm%c" : "\033[38;2;%03d;%03d;%03dm%c";

    const int tiling_string_length = strlen(tiling_string);
    int index = 0;

    // Calculate safe buffer size
    // 25 characters per pixel (including escape codes), 6 characters for newline and reset, 10 characters for final reset and null terminator
    size_t buffer_size = (img->width * img->height * 25) + (img->height * 6) + 5;
    char* output = malloc(buffer_size);
    if (!output) {
        perror("Failed to allocate output buffer");
        return NULL;
    }

    size_t pos = 0; // To track osition in output buffer

    for (int y = 0; y < img->height; y++) {
        for(int x = 0; x < img->width; x++) {
            const RGBAColor color = get_rgba_color(img, x, y);
            int written;

            if(color.a == 0){
                // No need to use color for transparent pixels, adds a space instead
                written = snprintf(&output[pos], buffer_size - pos, "\033[0m ");
            }else if(color.a == 255){
                // Write color and character to output buffer
                // Use tiling string for opaque pixels
                written = snprintf(&output[pos], buffer_size - pos, format_string, color.r, color.g, color.b, tiling_string[index++ % tiling_string_length]);
            }else{
                // Use ramp for semi-transparent pixels
                written = snprintf(&output[pos], buffer_size - pos, format_string, color.r, color.g, color.b, ramp.characters[color.a * (ramp.length-1) / 255]);
            }
            
    
            // Check for buffer overflow
            if (written < 0 || written >= buffer_size - pos) {
                fprintf(stderr, "Buffer overflow when writing to output\n");
                free(output);
                return NULL;
            }

            pos += written;
        }
        int written = snprintf(output + pos, buffer_size - pos, "\033[0m\n"); // Reset color and newline
        pos += written;
    }
    snprintf(output + pos, buffer_size - pos, "\033[0m");

    return output;
}