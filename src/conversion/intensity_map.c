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

    size_t index = 0; // Position in output buffer
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            const uint8_t* pixel = image_pixel_at(img, x, y);
            const uint8_t intensity = pixel[0];
            const int ramp_index = intensity * (ramp_length-1) / 255;
            
            // Get position and length of UTF-8 character in the ramp
            int char_pos = 0;
            for (int i = 0; i < ramp_index; i++) {
                int char_len = ascii_ramp_char_length(ramp->characters, i);
                char_pos += char_len;
            }
            
            // Get current character length
            int char_len = ascii_ramp_char_length(ramp->characters, ramp_index);
            
            // Copy the UTF-8 character to output
            memcpy(&output[index], &ramp->characters[char_pos], char_len);
            index += char_len;
        }
        output[index++] = '\n';
    }
    output[index] = '\0';

    return output;
}