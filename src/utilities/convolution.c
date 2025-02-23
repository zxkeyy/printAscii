#include <stdio.h>
#include <stdlib.h>
#include "utilities/convolution.h"
#include "utilities/clamp.h"

void convolution(const int* input, int input_height, int input_width,
                 int* output, const int* kernel, int kernel_size, int divisor) {
    // Validate input parameters
    if (kernel_size % 2 == 0 || kernel_size < 1) {
        fprintf(stderr, "Kernel size must be a positive odd number\n");
        return;
    }
    if (divisor == 0) {
        fprintf(stderr, "Divisor cannot be zero, using 1 instead\n");
        divisor = 1;
    }
    
    // Calculate kernel radius
    const int offset = kernel_size / 2;
    
    // Process all pixels
    for (int y = 0; y < input_height; y++) {
        for (int x = 0; x < input_width; x++) {
            int sum = 0;
            
            // Apply kernel to neighborhood
            for (int ky = 0; ky < kernel_size; ky++) {
                for (int kx = 0; kx < kernel_size; kx++) {
                    // Calculate image coordinates with extension
                    int img_y = y + ky - offset;
                    int img_x = x + kx - offset;

                    // Extend borders
                    img_x = clamp(img_x, 0, input_width - 1);
                    img_y = clamp(img_y, 0, input_height - 1);
                    
                    sum += kernel[ky * kernel_size + kx] * input[img_y * input_width + img_x];
                }
            }
            
            // Apply normalization and store result
            output[y * input_width + x] = sum / divisor;
        }
    }
}