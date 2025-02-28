#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "conversion/image_to_ansi.h"

char* image_to_ansi(Image* img, char* string, uint8_t background_brightness){
    switch(img->type) {
        case IMAGE_TYPE_GRAY: return grayscale_image_to_ansi(img, string);
        case IMAGE_TYPE_RGB: return RGB_image_to_ansi(img, string);
        case IMAGE_TYPE_RGBA: return RGBA_image_to_ansi(img, string, background_brightness);
        default: fprintf(stderr, "Unsupported image type\n");
    }
}

char* grayscale_image_to_ansi(Image* img, char* string){
    if (img == NULL) {
        fprintf(stderr, "Image is NULL\n");
        return NULL;
    }
    if(img->type != IMAGE_TYPE_GRAY) {
        fprintf(stderr, "Image is not grayscale\n");
        return NULL;
    }

    const char ANSI_COLORED_CHAR[] = "\033[38;2;%03d;%03d;%03dm%c";

    const int string_length = strlen(string);
    int index = 0;

    char* output = malloc((img->width * img->height) * (strlen(ANSI_COLORED_CHAR) + 1) + img->height * 5 + 5);
    if (!output) {
        perror("Failed to allocate output buffer");
        return NULL;
    }
    output[0] = '\0';

    char* colored_char = malloc(20);
    if (!colored_char) {
        perror("Failed to allocate colored_char buffer");
        free(output);
        return NULL;
    }

    for (int y = 0; y < img->height; y++){
        for(int x = 0; x < img->width; x++){
            const uint8_t* pixel = image_pixel_at(img, x, y);
            uint8_t brightness = pixel[0];

            snprintf(colored_char, 21, ANSI_COLORED_CHAR, (int) brightness, (int) brightness, (int) brightness, string[index++]);
            //printf("%s\n", colored_char);
            strcat(output, colored_char);

            if (index >= string_length){
                index = 0;
            }
        }
        strcat(output, "\033[0m\n");
    }

    strcat(output, "\033[0m");
    
    free(colored_char);
    
    return output;
}

char* RGB_image_to_ansi(Image* img, char* string){
    if (img == NULL) {
        fprintf(stderr, "Image is NULL\n");
        return NULL;
    }
    if(img->type != IMAGE_TYPE_RGB) {
        fprintf(stderr, "Image is not RGB\n");
        return NULL;
    }

    const char ANSI_COLORED_CHAR[] = "\033[48;2;%03d;%03d;%03dm%c";

    const int string_length = strlen(string);
    int index = 0;

    printf("image width: %d, imag height: %d\n", img->width, img->height);
    char* output = malloc((img->width * img->height) * (strlen(ANSI_COLORED_CHAR) + 1) + img->height * 5 + 5);
    if (!output) {
        perror("Failed to allocate output buffer");
        return NULL;
    }
    output[0] = '\0';

    char* colored_char = malloc(20);
    if (!colored_char) {
        perror("Failed to allocate colored_char buffer");
        free(output);
        return NULL;
    }

    for (int y = 0; y < img->height; y++){
        for(int x = 0; x < img->width; x++){
            const uint8_t* pixel = image_pixel_at(img, x, y);
            uint8_t red = pixel[0];
            uint8_t green = pixel[1];
            uint8_t blue = pixel[2];

            snprintf(colored_char, 21, ANSI_COLORED_CHAR, (int) red, (int) green, (int) blue, string[index++]);
            //colored_char[strlen(colored_char) + 1] = '\0';
            //printf("%s\n", colored_char);
            strcat(output, colored_char);

            if (index >= string_length){
                index = 0;
            }
        }
        strcat(output, "\033[0m\n");
    }

    strcat(output, "\033[0m");
    
    free(colored_char);
    
    return output;
}

char* RGBA_image_to_ansi(Image* img, char* string, uint8_t background_brightness){
    if (img == NULL) {
        fprintf(stderr, "Image is NULL\n");
        return NULL;
    }
    if(img->type != IMAGE_TYPE_RGBA) {
        fprintf(stderr, "Image is not RGBA\n");
        return NULL;
    }

    const char ANSI_COLORED_CHAR[] = "\033[48;2;%03d;%03d;%03dm%c";

    printf("image width: %d, imag height: %d\n", img->width, img->height);
    char* output = malloc((img->width * img->height) * (strlen(ANSI_COLORED_CHAR) + 1) + img->height * 5 + 5);
    if (!output) {
        perror("Failed to allocate output buffer");
        return NULL;
    }
    output[0] = '\0';

    char* colored_char = malloc(20);
    if (!colored_char) {
        perror("Failed to allocate colored_char buffer");
        free(output);
        return NULL;
    }

    const int string_length = strlen(string);
    int index = 0;
    for (int y = 0; y < img->height; y++){
        for(int x = 0; x < img->width; x++){
            const uint8_t* pixel = image_pixel_at(img, x, y);
            uint8_t alpha = pixel[3];
            uint8_t red = (int) pixel[0] * alpha / 255 + (int) background_brightness * (255 - alpha) / 255;
            uint8_t green = (int) pixel[1] * alpha / 255 + (int) background_brightness * (255 - alpha) / 255;
            uint8_t blue = (int) pixel[2] * alpha / 255 + (int) background_brightness * (255 - alpha) / 255;

            snprintf(colored_char, 21, ANSI_COLORED_CHAR, (int) red, (int) green, (int) blue, string[index++]);
            //colored_char[strlen(colored_char) + 1] = '\0';
            //printf("%s\n", colored_char);
            strcat(output, colored_char);

            if (index >= string_length){
                index = 0;
            }
        }
        strcat(output, "\033[0m\n");
    }

    strcat(output, "\033[0m");
    
    free(colored_char);
    
    return output;
}