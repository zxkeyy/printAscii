#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "preprocessing/gaussian_blur.h"
#include "utilities/clamp.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void gaussian_blur(Image* img, float sigma){
    if (img == NULL) {
        fprintf(stderr, "Image is NULL\n");
        return;
    }
    if (img->type != IMAGE_TYPE_GRAY) {
        fprintf(stderr, "Image is not grayscale\n");
        return;
    }

    const int kernel_size = 2 * (int)(2 * sigma) + 3;
    const int half_kernel = kernel_size / 2;
    const float sigma2 = sigma * sigma;

    // Allocate memory for the kernel
    float* kernel = malloc(kernel_size * kernel_size * sizeof(float));
    if(!kernel){
        perror("Failed to allocate kernel buffer");
        return;
    }

    // Generate the kernel
    float sum = 0;
    for(int y = -half_kernel; y <= half_kernel; y++){
        for(int x = -half_kernel; x <= half_kernel; x++){
            const float value = exp(-(x * x + y * y) / (2 * sigma2)) / (2 * M_PI * sigma2);
            kernel[(y + half_kernel) * kernel_size + (x + half_kernel)] = value;
            sum += value;
        }
    }

    // Normalize the kernel
    for(int i = 0; i < kernel_size * kernel_size; i++){
        kernel[i] /= sum;
    }

    // Apply the convolution
    Image* output = image_create(img->width, img->height, img->type);
    if(!output){
        fprintf(stderr, "Failed to allocate output image\n");
        free(kernel);
        return;
    }

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            float sum = 0;
            for (int ky = -half_kernel; ky <= half_kernel; ky++) {
                for (int kx = -half_kernel; kx <= half_kernel; kx++) {
                    // Clamp coordinates to extend the border
                    int clamped_x = clamp(x + kx, 0, img->width - 1);
                    int clamped_y = clamp(y + ky, 0, img->height - 1);

                    const uint8_t* pixel = image_pixel_at(img, clamped_x, clamped_y);
                    float weight = kernel[(ky + half_kernel) * kernel_size + (kx + half_kernel)];
                    sum += weight * (*pixel);
                }
            }
            uint8_t* out_pixel = image_pixel_at(output, x, y);
            *out_pixel = (uint8_t)sum;
        }
    }

    free(kernel);

    free(img->pixels);
    img->pixels = output->pixels;
    output->pixels = NULL;
    image_free(output);

    return;

}