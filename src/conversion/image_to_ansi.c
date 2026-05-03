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

    // Calculate total ramp length once
    int ramp_length = ascii_ramp_total_chars(ramp.characters);
    
    // Pre-calculate ramp character positions and lengths to avoid quadratic O(N^2) overhead
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
                int ramp_index = color.a * (ramp_length-1) / 255;
                int char_pos = ramp_char_pos[ramp_index];
                int char_len = ramp_char_len[ramp_index];
                
                // Get current character as a multi-byte sequence
                char utf8_char[MAX_UTF8_CHAR_SIZE] = {0};
                memcpy(utf8_char, &ramp.characters[char_pos], char_len);
                
                // Write color and UTF-8 character to output buffer
                written = snprintf(&output[pos], buffer_size - pos, format_string, color.r, color.g, color.b, utf8_char[0]);
                // If character is multi-byte, we need to append the rest of the bytes manually
                if (char_len > 1) {
                    memcpy(&output[pos + written], &utf8_char[1], char_len - 1);
                    written += (char_len - 1);
                }
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