#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "conversion/image_to_ansi.h"

char* grayscale_image_to_ansi(Image* img, char* string){
    if (img == NULL) {
        fprintf(stderr, "Image is NULL\n");
        return NULL;
    }
    if(img->type != IMAGE_TYPE_GRAY) {
        fprintf(stderr, "Image is not grayscale\n");
        return NULL;
    }

    const char ANSI_COLORED_CHAR[] = "\033[38;2;%03d;%03d;%03dm ";

    const int string_length = strlen(string);
    int index = 0;

    char* output = malloc((img->width * img->height) * (strlen(ANSI_COLORED_CHAR) + 1) + img->height + 5);
    if (!output) {
        perror("Failed to allocate output buffer");
        return NULL;
    }

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

            snprintf(colored_char, 20, ANSI_COLORED_CHAR, (int) brightness, (int) brightness, (int) brightness);
            if (strlen(colored_char) == 19) {
                colored_char[strlen(colored_char)] = string[index++];
                colored_char[strlen(colored_char) + 1] = '\0';
            }
            //printf("%s\n", colored_char);
            strcat(output, colored_char);

            if (index >= string_length){
                index = 0;
            }
        }
        strcat(output, "\n");
    }

    strcat(output, "\033[0m");
    
    free(colored_char);
    
    return output;
}