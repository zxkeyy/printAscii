#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "core/image.h"

void sobel_edge_detection(Image* img, uint16_t threshold){
    if (img->type != IMAGE_TYPE_GRAY) {
        fprintf(stderr, "Image is not grayscale\n");
        return;
    }
    
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

// #include "utilities/convolution.h"
// void sobel_edge_detection(Image* img, uint16_t threshold) {
//     // Sobel operator kernels (flattened for `convolution` function)
//     const int kernel_x[9] = { 
//         -1, 0, 1, 
//         -2, 0, 2, 
//         -1, 0, 1 
//     };
//     const int kernel_y[9] = { 
//          1,  2,  1, 
//          0,  0,  0, 
//         -1, -2, -1 
//     };

//     // Allocate memory for gradient images
//     int* gradient_x = calloc(img->width * img->height, sizeof(int));
//     int* gradient_y = calloc(img->width * img->height, sizeof(int));
//     if (!gradient_x || !gradient_y) {
//         fprintf(stderr, "Failed to allocate gradient images\n");
//         if (gradient_x) free(gradient_x);
//         if (gradient_y) free(gradient_y);
//         return;
//     }
    
//     // Initialize gradient values to image pixel values
//     for (int i = 0; i < img->width * img->height; i++) {
//         uint8_t pixel_value = *image_pixel_at(img, i % img->width, i / img->width);
//         gradient_x[i] = pixel_value;
//         gradient_y[i] = pixel_value;
//     }

//     // Apply Sobel X and Y convolution
//     convolution(gradient_x, img->height, img->width, kernel_x, 3);
//     convolution(gradient_y, img->height, img->width, kernel_y, 3);

//     // Compute gradient magnitude
//     for (int y = 0; y < img->height; y++) {
//         for (int x = 0; x < img->width; x++) {
//             int gx = gradient_x[x + y * img->width];
//             int gy = gradient_y[x + y * img->width];

//             int magnitude = sqrt(gx * gx + gy * gy);
//             uint8_t* out_pixel = image_pixel_at(img, x, y);
//             *out_pixel = (magnitude > threshold) ? 255 : 0;
//         }
//     }

//     // Free the allocated memory for gradient
//     free(gradient_x);
//     free(gradient_y);
// }