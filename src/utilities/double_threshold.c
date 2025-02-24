#include <stdio.h>
#include "utilities/double_threshold.h"

void double_threshold(Image* img, const uint8_t high_threshold, uint8_t low_threshold){
    if (img == NULL) {
        fprintf(stderr, "Image is NULL\n");
        return;
    }
    if(img->type != IMAGE_TYPE_GRAY) {
        fprintf(stderr, "Image is not grayscale\n");
        return;
    }

    for(int y = 0; y < img->height; y++) {
        for(int x = 0; x < img->width; x++) {
            uint8_t* pixel = image_pixel_at(img, x, y);
            if (*pixel >= high_threshold)
            {
                *pixel = 255; // Strong edge
            } else if (*pixel >= low_threshold)
            {
                *pixel = 128; // Weak edge
            } else {
                *pixel = 0; // Non edge
            }
        }
    }
}