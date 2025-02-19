#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "core/image.h"

void sobel_edge_detection(Image* img, uint16_t threshold){
    // Sobel operator kernels
    const int kernel_x[3][3] = {
        { -1, 0, 1 },
        { -2, 0, 2 },
        { -1, 0, 1 }
    };
    const int kernel_y[3][3] = {
        { 1, 2, 1 },
        { 0, 0, 0 },
        { -1, -2, -1 }
    };

    // Allocate memory for the output image
    Image* output = image_create(img->width, img->height, IMAGE_TYPE_GRAY);
    if (!output) {
        fprintf(stderr, "Failed to allocate output image\n");
        return;
    }

    // Apply the Sobel operator to the image
    for (int y = 1; y < img->height - 1; y++) {
        for (int x = 1; x < img->width - 1; x++) {
            int gx = 0, gy = 0;
            for (int ky = 0; ky < 3; ky++) {
                for (int kx = 0; kx < 3; kx++) {
                    const uint8_t* pixel = image_pixel_at(img, x + kx - 1, y + ky - 1);
                    gx += kernel_x[ky][kx] * *pixel;
                    gy += kernel_y[ky][kx] * *pixel;
                }
            }
            uint8_t* out_pixel = image_pixel_at(output, x, y);
            *out_pixel = (uint8_t)((uint16_t)sqrt(gx * gx + gy * gy) > threshold ? 255 : 0);
        }
    }

    free(img->pixels);
    img->pixels = output->pixels;
    img->type = IMAGE_TYPE_GRAY;
    output->pixels = NULL;
    image_free(output);
}