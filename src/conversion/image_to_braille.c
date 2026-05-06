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
    
    int channels = img->channels;
    int row_stride = img->width * channels;
    uint8_t* pixels = img->pixels;

    // Precalculate boundary limits
    int safe_width = img->width & ~1;  // Even width
    int safe_height = img->height & ~3; // Multiple of 4 height
    
    int row1 = row_stride;
    int row2 = row_stride * 2;
    int row3 = row_stride * 3;
    int ch2 = channels;

    for (int y = 0; y < img->height; y += 4){
        uint8_t* row_base = pixels + y * row_stride;
        for(int x = 0; x < img->width; x += 2){
            int offset = 0; 
            
            // Fast path: fully unrolled, no boundary checks
            if (y < safe_height && x < safe_width) {
                uint8_t* p = row_base + x * channels;
                if (p[0] > threshold) offset |= 1;           // j=0, i=0
                if (p[row1] > threshold) offset |= 2;        // j=1, i=0
                if (p[row2] > threshold) offset |= 4;        // j=2, i=0
                if (p[row3] > threshold) offset |= 64;       // j=3, i=0
                
                if (p[ch2] > threshold) offset |= 8;         // j=0, i=1
                if (p[row1 + ch2] > threshold) offset |= 16;   // j=1, i=1
                if (p[row2 + ch2] > threshold) offset |= 32;   // j=2, i=1
                if (p[row3 + ch2] > threshold) offset |= 128;  // j=3, i=1
            } 
            // Fallback for right/bottom edges where we might hit bounds
            else {
                for (int i = 0; i < 2; i++){
                    for (int j = 0; j < 4; j++){
                        if (x + i < img->width && y + j < img->height){
                            if (pixels[(y + j) * row_stride + (x + i) * channels] > threshold){
                                offset |= (1 << braille_map[j][i]);
                            }
                        }
                    }
                }
            }
            
            // Convert to UTF-8. 
            // Braille base 0x2800 is always 3 bytes starting with 11100010 (0xE2).
            // Format: 11100010 101000dd 10dddddd (where d is the offset pattern).
            output[pos++] = (char)0xE2;
            output[pos++] = (char)(0xA0 | (offset >> 6));
            output[pos++] = (char)(0x80 | (offset & 0x3F));
        }
        output[pos++] = '\n';
    }
    output[pos] = '\0';

    return output;
}