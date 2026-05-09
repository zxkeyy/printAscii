#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "conversion/image_to_ansi.h"
#include "core/ascii_ramp.h"

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

char* image_to_ansi(Image* img, char* tiling_string, RGBColor background_color, int background){
    if (img == NULL) {
        fprintf(stderr, "Image is NULL\n");
        return NULL;
    }

    // Parse tiling_string into UTF-8-aware tiles (positions + lengths)
    const int tiling_string_len = (int)strlen(tiling_string);
    int tile_pos[1024];
    int tile_len[1024];
    int tile_count = 0;
    int tile_max_len = 1;
    int ti = 0;
    while (ti < tiling_string_len && tile_count < 1024) {
        unsigned char c = (unsigned char)tiling_string[ti];
        int l = 1;
        if ((c & 0xE0) == 0xC0) l = 2;
        else if ((c & 0xF0) == 0xE0) l = 3;
        else if ((c & 0xF8) == 0xF0) l = 4;
        if (ti + l > tiling_string_len) break;
        tile_pos[tile_count] = ti;
        tile_len[tile_count] = l;
        if (l > tile_max_len) tile_max_len = l;
        tile_count++;
        ti += l;
    }
    if (tile_count == 0) {
        // fallback to single ASCII space if tiling string is empty/invalid
        tiling_string = " ";
        tile_pos[0] = 0;
        tile_len[0] = 1;
        tile_count = 1;
        tile_max_len = 1;
    }
    int tile_index = 0;

    // Calculate safe buffer size
    // 25 characters per pixel (including escape codes), plus space for multi-byte tiles
    size_t buffer_size = (img->width * img->height * (25 + (tile_max_len - 1))) + (img->height * 6) + 15;
    char* output = malloc(buffer_size);
    if (!output) {
        perror("Failed to allocate output buffer");
        return NULL;
    }

    int channels = img->channels;
    int row_stride = img->width * channels;
    uint8_t* pixels = img->pixels;
    char* p = output;

    for (int y = 0; y < img->height; y++) {
        uint8_t* curr_row = pixels + y * row_stride;
        int prev_r = -1, prev_g = -1, prev_b = -1;

        for(int x = 0; x < img->width; x++) {
            uint8_t* pixel = curr_row + x * channels;
            
            uint8_t r, g, b;
            if (img->type == IMAGE_TYPE_RGBA) {
                uint8_t alpha = pixel[3];
                r = (pixel[0] * alpha / 255) + (background_color.r * (255 - alpha) / 255);
                g = (pixel[1] * alpha / 255) + (background_color.g * (255 - alpha) / 255);
                b = (pixel[2] * alpha / 255) + (background_color.b * (255 - alpha) / 255);
            } else if (channels >= 3) {
                r = pixel[0];
                g = pixel[1];
                b = pixel[2];
            } else {
                r = g = b = pixel[0];
            }
            
            if (r != prev_r || g != prev_g || b != prev_b) {
                p = fast_append_color(p, r, g, b, background);
                prev_r = r; prev_g = g; prev_b = b;
            }
            
            // Append next UTF-8 tile
            int tl = tile_len[tile_index];
            int tpos = tile_pos[tile_index];
            for (int kk = 0; kk < tl; kk++) {
                *p++ = tiling_string[tpos + kk];
            }
            tile_index = (tile_index + 1) % tile_count;
    
            if ((size_t)(p - output) > buffer_size - 60) {
                fprintf(stderr, "Buffer overflow when writing to output\n");
                free(output);
                return NULL;
            }
        }
        *p++ = '\033'; *p++ = '['; *p++ = '0'; *p++ = 'm'; *p++ = '\n';
    }
    *p++ = '\033'; *p++ = '['; *p++ = '0'; *p++ = 'm'; *p++ = '\0';

    return output;
}

char* image_to_alpha_ansi(Image* img, char* tiling_string, AsciiRamp ramp, int background){
    if (img == NULL) {
        fprintf(stderr, "Image is NULL\n");
        return NULL;
    }

    // Parse tiling_string into UTF-8-aware tiles for alpha-aware ANSI output
    const int tiling_string_len2 = (int)strlen(tiling_string);
    int tile_pos2[1024];
    int tile_len2[1024];
    int tile_count2 = 0;
    int tile_max_len2 = 1;
    int ti2 = 0;
    while (ti2 < tiling_string_len2 && tile_count2 < 1024) {
        unsigned char c = (unsigned char)tiling_string[ti2];
        int l = 1;
        if ((c & 0xE0) == 0xC0) l = 2;
        else if ((c & 0xF0) == 0xE0) l = 3;
        else if ((c & 0xF8) == 0xF0) l = 4;
        if (ti2 + l > tiling_string_len2) break;
        tile_pos2[tile_count2] = ti2;
        tile_len2[tile_count2] = l;
        if (l > tile_max_len2) tile_max_len2 = l;
        tile_count2++;
        ti2 += l;
    }
    if (tile_count2 == 0) {
        tiling_string = " ";
        tile_pos2[0] = 0;
        tile_len2[0] = 1;
        tile_count2 = 1;
        tile_max_len2 = 1;
    }
    int tile_index2 = 0;

    size_t buffer_size = (img->width * img->height * (25 + (tile_max_len2 - 1))) + (img->height * 6) + 15;
    char* output = malloc(buffer_size);
    if (!output) {
        perror("Failed to allocate output buffer");
        return NULL;
    }

    int ramp_length = ascii_ramp_total_chars(ramp.characters);
    
    int ramp_char_pos[1024];
    int ramp_char_len[1024];
    int current_pos = 0;
    for (int i = 0; i < ramp_length; i++) {
        unsigned char c = (unsigned char)ramp.characters[current_pos];
        int len = 1;
        if ((c & 0xE0) == 0xC0) len = 2;
        else if ((c & 0xF0) == 0xE0) len = 3;
        else if ((c & 0xF8) == 0xF0) len = 4;
        
        ramp_char_pos[i] = current_pos;
        ramp_char_len[i] = len;
        current_pos += len;
    }

    int channels = img->channels;
    int row_stride = img->width * channels;
    uint8_t* pixels = img->pixels;
    char* p = output;

    for (int y = 0; y < img->height; y++) {
        uint8_t* curr_row = pixels + y * row_stride;
        int prev_r = -1, prev_g = -1, prev_b = -1;

        for(int x = 0; x < img->width; x++) {
            uint8_t* pixel = curr_row + x * channels;
            
            uint8_t r, g, b, a;
            if (img->type == IMAGE_TYPE_RGBA) {
                r = pixel[0]; g = pixel[1]; b = pixel[2]; a = pixel[3];
            } else if (channels >= 3) {
                r = pixel[0]; g = pixel[1]; b = pixel[2]; a = 255;
            } else {
                r = g = b = pixel[0]; a = 255;
            }

            if(a == 0){
                *p++ = '\033'; *p++ = '['; *p++ = '0'; *p++ = 'm'; *p++ = ' ';
                prev_r = -1; prev_g = -1; prev_b = -1;
            }else{
                if (r != prev_r || g != prev_g || b != prev_b) {
                    p = fast_append_color(p, r, g, b, background);
                    prev_r = r; prev_g = g; prev_b = b;
                }
                
                if(a == 255){
                    int tl = tile_len2[tile_index2];
                    int tpos = tile_pos2[tile_index2];
                    for (int kk = 0; kk < tl; kk++) {
                        *p++ = tiling_string[tpos + kk];
                    }
                    tile_index2 = (tile_index2 + 1) % tile_count2;
                }else{
                    int ramp_index = a * (ramp_length-1) / 255;
                    int char_pos = ramp_char_pos[ramp_index];
                    int char_len = ramp_char_len[ramp_index];
                    
                    for(int c=0; c<char_len; c++) {
                        *p++ = ramp.characters[char_pos + c];
                    }
                }
            }
    
            if ((size_t)(p - output) > buffer_size - 60) {
                fprintf(stderr, "Buffer overflow when writing to output\n");
                free(output);
                return NULL;
            }
        }
        *p++ = '\033'; *p++ = '['; *p++ = '0'; *p++ = 'm'; *p++ = '\n';
    }
    *p++ = '\033'; *p++ = '['; *p++ = '0'; *p++ = 'm'; *p++ = '\0';

    return output;
}