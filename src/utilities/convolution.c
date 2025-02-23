#include <stdio.h>
#include <stdlib.h>
#include "utilities/convolution.h"

void convolution(int* input, int input_height, int input_width, const int* kernel, int kernel_size) {
    if (kernel_size % 2 == 0) {
        fprintf(stderr, "Error in convolution: Kernel size must be an odd number\n");
        return;
    }

    // Allocate memory for the output buffer
    int* output = malloc(input_height * input_width * sizeof(int));

    if(!output){
        perror("Failed to allocate output buffer");
        return;
    }

    const int offset = kernel_size / 2;

    // Apply the convolution to the image
    for (int y = offset; y < input_height - offset; y++) {
        for (int x = offset; x < input_width - offset; x++) {
            int sum = 0;
            for (int ky = 0; ky < kernel_size; ky++) {
                for (int kx = 0; kx < kernel_size; kx++) {
                    sum += kernel[ky * kernel_size + kx] * input[(y + ky - offset) * input_width + (x + kx - offset)];
                }
            }
            output[ y * input_width + x ]  = sum;
        }
    }

    // Copy the output buffer back to the input buffer
    for (int i = 0; i < input_height * input_width; i++) {
        input[i] = output[i];
    }

    free(output);    
}