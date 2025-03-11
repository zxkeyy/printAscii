#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "conversion/image_to_html.h"

char* image_to_html(Image* img, char* tiling_string, RGBColor background_color, int background){
    if (img == NULL) {
        fprintf(stderr, "Image is NULL\n");
        return NULL;
    }

    // HTML span tag for colored character, uses 24-bit color
    const char* format_string = background ? "<span style='background-color: rgb(%03d, %03d, %03d);'>%c</span>" : "<span style='color: rgb(%03d, %03d, %03d);'>%c</span>";

    const int tiling_string_length = strlen(tiling_string);
    int index = 0;

    // Calculate safe buffer size
    // 64 characters per pixel , 4 characters for newline (/<br>), 9 characters for start and end <pre> tags, 1 character for null terminator
    size_t buffer_size = (img->width * img->height * 64) + (img->height * 4) + 10;
    char* output = malloc(buffer_size);
    if (!output) {
        perror("Failed to allocate output buffer");
        return NULL;
    }

    size_t pos = 0; // To track position in output buffer

    // Write start of pre tag
    int written = snprintf(&output[pos], buffer_size - pos, "<pre>");
    pos += written;

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
        int written = snprintf(output + pos, buffer_size - pos, "<br>"); // Newline
        pos += written;
    }
    snprintf(output + pos, buffer_size - pos, "</pre>");

    return output;
}