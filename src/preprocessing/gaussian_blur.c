#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "preprocessing/gaussian_blur.h"
#include "utilities/convolution.h"

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
    // Allocate temporary buffers to store values as integers
    int* input_buffer = malloc(img->width * img->height * sizeof(int));
    int* output_buffer = malloc(img->width * img->height * sizeof(int));
    if(!input_buffer || !output_buffer){
        fprintf(stderr, "Failed to allocate convolution buffers\n");
        free(kernel);
        if(input_buffer) free(input_buffer);
        if(output_buffer) free(output_buffer);
        return;
    }

    // Initialize buffer to image pixels values
    for (int i = 0; i < img->width * img->height; i++)
    {
        input_buffer[i] = (int) img->pixels[i];
    }
    
    convolution(input_buffer, img->height, img->width, output_buffer, kernel, kernel_size, 1);

    // Copy result to image pixels
    for (int i = 0; i < img->width * img->height; i++)
    {
        img->pixels[i] = (uint8_t) output_buffer[i];
    }

    free(input_buffer);
    free(output_buffer);
    free(kernel);
    return;
}