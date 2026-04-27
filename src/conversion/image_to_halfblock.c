#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "conversion/image_to_halfblock.h"

char* image_to_halfblock(Image* img) {
    if (img == NULL) {
        fprintf(stderr, "Image is NULL\n");
        return NULL;
    }

    // Calculate safe buffer size.
    // Each block contains \033[38;2;R;G;Bm (foreground)
    // and \033[48;2;R;G;Bm (background), then ▀ (3 bytes).
    // Around 43 characters per block maximally + 6 for newline reset + 10 padding.
    // Notice that img->height is in pixels; halfblocks process 2 pixel vertical slices at a time.
    int output_rows = (img->height + 1) / 2;
    size_t buffer_size = (img->width * output_rows * 45) + (output_rows * 8) + 15;
    char* output = malloc(buffer_size);
    if (!output) {
        perror("Failed to allocate output buffer");
        return NULL;
    }

    size_t pos = 0; // To track position in output buffer

    // Format: foreground color (top pixel), background color (bottom pixel), Unicode top half-block '▀'
    const char* format_string = "\033[38;2;%d;%d;%dm\033[48;2;%d;%d;%dm\xE2\x96\x80";

    for (int y = 0; y < img->height; y += 2) {
        for(int x = 0; x < img->width; x++) {
            // Get the top pixel color
            RGBAColor top_color = get_rgba_color(img, x, y);
            uint8_t top_r = top_color.r;
            uint8_t top_g = top_color.g;
            uint8_t top_b = top_color.b;
            
            // Get the bottom pixel color (if we're not at the very bottom edge)
            uint8_t bot_r = 0, bot_g = 0, bot_b = 0;
            if (y + 1 < img->height) {
                RGBAColor bot_color = get_rgba_color(img, x, y + 1);
                bot_r = bot_color.r;
                bot_g = bot_color.g;
                bot_b = bot_color.b;
            }

            int written = snprintf(&output[pos], buffer_size - pos, format_string, 
                                   top_r, top_g, top_b, 
                                   bot_r, bot_g, bot_b);
            
            if (written < 0 || (size_t)written >= buffer_size - pos) {
                fprintf(stderr, "Buffer overflow when writing to output\n");
                free(output);
                return NULL;
            }
            pos += written;
        }

        // Reset color and append newline
        int written = snprintf(output + pos, buffer_size - pos, "\033[0m\n");
        
        if (written < 0 || (size_t)written >= buffer_size - pos) {
            fprintf(stderr, "Buffer overflow when writing to output\n");
            free(output);
            return NULL;
        }
        pos += written;
    }
    
    snprintf(output + pos, buffer_size - pos, "\033[0m");
    return output;
}
