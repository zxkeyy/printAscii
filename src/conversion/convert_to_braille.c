#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "conversion/convert_to_braille.h"

int16_t* convert_to_braille(Image* img, int threshold){
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
    
    int16_t* output = malloc(((img->width / 2) * (img->height / 4) + img->height + 1) * sizeof(int16_t));
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