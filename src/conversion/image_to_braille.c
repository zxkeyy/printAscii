#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "conversion/image_to_braille.h"

char* image_to_braille(Image* img, int threshold){
    if (img == NULL) {
        fprintf(stderr, "Image is NULL\n");
        return NULL;
    }
    if(img->type != IMAGE_TYPE_GRAY) {
        fprintf(stderr, "Image is not grayscale\n");
        return NULL;
    }

    static const int braille_map[4][2] = {
        {0, 3},
        {1, 4},
        {2, 5},
        {6, 7}
    };
    
    // Each braille character takes up to 3 bytes in UTF-8, plus 1 for newline and 1 for null terminator
    // Allocate enough memory for the worst case
    size_t buffer_size = (((img->width + 1)/ 2) * 3 + 1) * ((img->height + 3) / 4) + 1;
    char* output = malloc(buffer_size);
    if (!output) {
        perror("Failed to allocate output buffer");
        return NULL;
    }

    size_t pos = 0; // Current position in the output buffer
    for (int y = 0; y < img->height; y += 4){
        for(int x = 0; x < img->width; x += 2){
            int codepoint = 0x2800; // Base codepoint for braille patterns
            for (int i = 0; i < 2; i++){
                for (int j = 0; j < 4; j++){
                    if (x + i >= img->width || y + j >= img->height){
                        continue;
                    }

                    const uint8_t* pixel = image_pixel_at(img, x + i, y + j);
                    if (pixel[0] > threshold){
                        codepoint += (1 << braille_map[j][i]);
                    }
                }
            }
            
            // Convert Unicode codepoint to UTF-8
            if (codepoint <= 0x7F) {
                // 1-byte UTF-8 (shouldn't happen for braille which is always > 0x7F)
                output[pos++] = (char)codepoint;
            } else if (codepoint <= 0x7FF) {
                // 2-byte UTF-8 (shouldn't happen for braille which is > 0x7FF)
                output[pos++] = (char)(0xC0 | (codepoint >> 6));
                output[pos++] = (char)(0x80 | (codepoint & 0x3F));
            } else if (codepoint <= 0xFFFF) {
                // 3-byte UTF-8 (braille patterns are in this range)
                output[pos++] = (char)(0xE0 | (codepoint >> 12));
                output[pos++] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
                output[pos++] = (char)(0x80 | (codepoint & 0x3F));
            } else {
                // 4-byte UTF-8 (shouldn't happen for braille)
                output[pos++] = (char)(0xF0 | (codepoint >> 18));
                output[pos++] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
                output[pos++] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
                output[pos++] = (char)(0x80 | (codepoint & 0x3F));
            }
        }
        output[pos++] = '\n';
    }
    output[pos] = '\0';

    return output;
}