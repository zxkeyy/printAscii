#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "core/image.h"
#include "preprocessing/sobel_edge_detection.h"
#include "utilities/sobel_operator.h"

bool sobel_edge_detection(Image* img, uint16_t threshold){
    if (!img) {
        fprintf(stderr, "Image is NULL\n");
        return false;
    }

    if (img->type != IMAGE_TYPE_GRAY) {
        fprintf(stderr, "Image is not grayscale\n");
        return false;
    }

    // Allocate memory for the output image
    Image* output = image_create(img->width, img->height, IMAGE_TYPE_GRAY);
    if (!output) {
        fprintf(stderr, "Failed to allocate output image\n");
        return false;
    }

    sobel_operator(img->pixels, output->pixels, NULL, img->width, img->height, threshold);

    free(img->pixels);
    img->pixels = output->pixels;
    output->pixels = NULL;
    image_free(output);
    return true;
}

// void sobel_edge_detection(Image* img, uint16_t threshold){
//     if (img->type != IMAGE_TYPE_GRAY) {
//         fprintf(stderr, "Image is not grayscale\n");
//         return;
//     }
    
//     // Sobel operator kernels
//     const int kernel_x[3][3] = {
//         { -1, 0, 1 },
//         { -2, 0, 2 },
//         { -1, 0, 1 }
//     };
//     const int kernel_y[3][3] = {
//         { 1, 2, 1 },
//         { 0, 0, 0 },
//         { -1, -2, -1 }
//     };

//     // Allocate memory for the output image
//     Image* output = image_create(img->width, img->height, IMAGE_TYPE_GRAY);
//     if (!output) {
//         fprintf(stderr, "Failed to allocate output image\n");
//         return;
//     }

//     // Apply the Sobel operator to the image
//     for (int y = 0; y < img->height; y++) {
//         for (int x = 0; x < img->width; x++) {
//             int gx = 0, gy = 0;
            
//             for (int ky = -1; ky <= 1; ky++) {
//                 for (int kx = -1; kx <= 1; kx++) {
//                     // Clamp coordinates to stay inside image
//                     int clamped_x = clamp(x + kx, 0, img->width - 1);
//                     int clamped_y = clamp(y + ky, 0, img->height - 1);

//                     const uint8_t* pixel = image_pixel_at(img, clamped_x, clamped_y);
//                     gx += kernel_x[ky + 1][kx + 1] * (*pixel);
//                     gy += kernel_y[ky + 1][kx + 1] * (*pixel);
//                 }
//             }

//             uint8_t* out_pixel = image_pixel_at(output, x, y);
//             *out_pixel = (uint8_t)((uint16_t)sqrt(gx * gx + gy * gy) > threshold ? 255 : 0);
//         }
//     }

//     free(img->pixels);
//     img->pixels = output->pixels;
//     output->pixels = NULL;
//     image_free(output);
// }