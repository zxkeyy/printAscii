#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "conversion/image_to_halfblock.h"

// they call me the optimiser
// equivalent to sprintf(buffer, "\033[38;2;%d;%d;%dm\033[48;2;%d;%d;%dm\xE2\x96\x80", top_r, top_g, top_b, bot_r, bot_g, bot_b);
static inline char* fast_append_color(char* p, uint8_t r, uint8_t g, uint8_t b, int is_bg) {
    *p++ = '\033'; *p++ = '['; 
    *p++ = is_bg ? '4' : '3'; *p++ = '8'; *p++ = ';'; *p++ = '2'; *p++ = ';';
    
    if (r >= 100) { *p++ = '0' + (r / 100); *p++ = '0' + ((r / 10) % 10); *p++ = '0' + (r % 10); }
    else if (r >= 10) { *p++ = '0' + (r / 10); *p++ = '0' + (r % 10); }
    else { *p++ = '0' + r; }
    *p++ = ';';
    
    if (g >= 100) { *p++ = '0' + (g / 100); *p++ = '0' + ((g / 10) % 10); *p++ = '0' + (g % 10); }
    else if (g >= 10) { *p++ = '0' + (g / 10); *p++ = '0' + (g % 10); }
    else { *p++ = '0' + g; }
    *p++ = ';';
    
    if (b >= 100) { *p++ = '0' + (b / 100); *p++ = '0' + ((b / 10) % 10); *p++ = '0' + (b % 10); }
    else if (b >= 10) { *p++ = '0' + (b / 10); *p++ = '0' + (b % 10); }
    else { *p++ = '0' + b; }
    *p++ = 'm';
    
    return p;
}

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

    int channels = img->channels;
    int row_stride = img->width * channels;
    uint8_t* pixels = img->pixels;

    for (int y = 0; y < img->height; y += 2) {
        uint8_t* top_row = pixels + y * row_stride;
        uint8_t* bot_row = (y + 1 < img->height) ? (pixels + (y + 1) * row_stride) : NULL;

        int prev_top_r = -1, prev_top_g = -1, prev_top_b = -1;
        int prev_bot_r = -1, prev_bot_g = -1, prev_bot_b = -1;
        char* p = &output[pos];

        for(int x = 0; x < img->width; x++) {
            uint8_t top_r, top_g, top_b;
            uint8_t* top_pixel = top_row + x * channels;
            
            if (channels >= 3) {
                top_r = top_pixel[0];
                top_g = top_pixel[1];
                top_b = top_pixel[2];
            } else {
                top_r = top_g = top_b = top_pixel[0];
            }
            
            uint8_t bot_r = 0, bot_g = 0, bot_b = 0;
            if (bot_row) {
                uint8_t* bot_pixel = bot_row + x * channels;
                if (channels >= 3) {
                    bot_r = bot_pixel[0];
                    bot_g = bot_pixel[1];
                    bot_b = bot_pixel[2];
                } else {
                    bot_r = bot_g = bot_b = bot_pixel[0];
                }
            }

            // Only append color if it changed (Deduplication reduces buffer & terminal payload)
            if (top_r != prev_top_r || top_g != prev_top_g || top_b != prev_top_b) {
                p = fast_append_color(p, top_r, top_g, top_b, 0);
                prev_top_r = top_r; prev_top_g = top_g; prev_top_b = top_b;
            }
            
            if (bot_r != prev_bot_r || bot_g != prev_bot_g || bot_b != prev_bot_b) {
                p = fast_append_color(p, bot_r, bot_g, bot_b, 1);
                prev_bot_r = bot_r; prev_bot_g = bot_g; prev_bot_b = bot_b;
            }

            // Append upper halfblock \xE2\x96\x80
            *p++ = '\xE2';
            *p++ = '\x96';
            *p++ = '\x80';

            // Check if remaining buffer gets dangerously close to maximum append boundary
            if ((size_t)(p - output) > buffer_size - 60) {
                fprintf(stderr, "Buffer overflow when writing to output\n");
                free(output);
                return NULL;
            }
        }

        // Reset color and append newline
        *p++ = '\033'; *p++ = '['; *p++ = '0'; *p++ = 'm'; *p++ = '\n';
        pos = p - output;
    }
    
    output[pos++] = '\033';
    output[pos++] = '[';
    output[pos++] = '0';
    output[pos++] = 'm';
    output[pos++] = '\0';
    
    return output;
}
