#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/image.h"
#include "core/ascii_ramp.h"
#include "conversion/intensity_map.h"

char* intensity_map(const Image* img, const AsciiRamp* ramp){
    if (!image_validate(img)) {
        fprintf(stderr, "Invalid image\n");
        return NULL;
    }
    if (!ascii_ramp_validate(ramp)) {
        fprintf(stderr, "Invalid ASCII ramp\n");
        return NULL;
    }

    const int ramp_length = ramp->length;
    
    // Calculate maximum buffer size needed (assuming worst case: 4 bytes per UTF-8 character)
    size_t max_char_size = 4;
    size_t buffer_size = (img->width * max_char_size * img->height) + img->height + 1;
    char* output = malloc(buffer_size);
    if (!output) {
        perror("Failed to allocate output buffer");
        return NULL;
    }

    // Pre-calculate ramp character positions and lengths to avoid quadratic O(N^2) overhead
    int ramp_char_pos[1024];
    int ramp_char_len[1024];
    int current_pos = 0;
    for (int i = 0; i < ramp_length; i++) {
        // Optimize: ascii_ramp_char_length with index 0 looks at the exact start of the string!
        // No, ascii_ramp_char_length does iteration to find the index.
        // We can just parse the UTF-8 bytes directly here to be O(N) total!
        unsigned char c = (unsigned char)ramp->characters[current_pos];
        int len = 1;
        if ((c & 0xE0) == 0xC0) len = 2;
        else if ((c & 0xF0) == 0xE0) len = 3;
        else if ((c & 0xF8) == 0xF0) len = 4;
        
        ramp_char_pos[i] = current_pos;
        ramp_char_len[i] = len;
        current_pos += len;
    }

    size_t index = 0; // Position in output buffer
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            const uint8_t* pixel = image_pixel_at(img, x, y);
            const uint8_t intensity = pixel[0];
            int ramp_index = intensity * ramp_length / 256;
            if (ramp_index >= ramp_length) {
                ramp_index = ramp_length - 1;
            }
            
            // Get position and length of UTF-8 character in the ramp using the cache
            int char_pos = ramp_char_pos[ramp_index];
            int char_len = ramp_char_len[ramp_index];
            
            // Copy the UTF-8 character to output
            memcpy(&output[index], &ramp->characters[char_pos], char_len);
            index += char_len;
        }
        output[index++] = '\n';
    }
    output[index] = '\0';

    return output;
}