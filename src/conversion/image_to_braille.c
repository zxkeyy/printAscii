#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "conversion/image_to_braille.h"

int16_t* image_to_braille(Image* img, int threshold){
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
    
    // Each braille character is 8 dots, so we need to divide the image into 2x4 blocks
    int16_t* output = malloc((((img->width + 1)/ 2) * ((img->height + 3) / 4) + (img->height + 3) /4 + 1) * sizeof(int16_t));
    if (!output) {
        perror("Failed to allocate output buffer");
        return NULL;
    }

    int index = 0;
    for (int y = 0; y < img->height; y += 4){
        for(int x = 0; x < img->width; x += 2){
            int16_t character = 0x2800;
            for (int i = 0; i < 2; i++){
                for (int j = 0; j < 4; j++){
                    if (x + i >= img->width || y + j >= img->height){
                        continue;
                    }

                    const uint8_t* pixel = image_pixel_at(img, x + i, y + j);
                    if (pixel[0] > threshold){
                        character += (int16_t)( 1 << braille_map[j][i]);
                    }
                }
            }
            output[index++] = character;
        }
        output[index++] = '\n';
    }
    output[index] = (int16_t)'\0';

    return output;
}