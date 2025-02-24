#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "utilities/sobel_operator.h"
#include "utilities/convolution.h"
#include "utilities/clamp.h"

void sobel_operator(const uint8_t* input, uint8_t* magnitude, float* angle, int width, int height, int threshold){

    // Sobel operator kernels
    const float kernel_x[9] = { 
        -1, 0, 1, 
        -2, 0, 2, 
        -1, 0, 1 
    };
    const float kernel_y[9] = { 
         1,  2,  1, 
         0,  0,  0, 
        -1, -2, -1 
    };

    // Allocate memory for gradient images
    int* gradient_x = calloc(width * height, sizeof(int));
    int* gradient_y = calloc(width * height, sizeof(int));
    if (!gradient_x || !gradient_y) {
        fprintf(stderr, "Failed to allocate gradient images\n");
        if (gradient_x) free(gradient_x);
        if (gradient_y) free(gradient_y);
        return;
    }

    // Cast input image pixels to int for compatibility with convolution function
    int* input_int = malloc(width * height * sizeof(int));
    if(!input_int) {
        perror("Failed to allocate memory for input image");
        free(gradient_x);
        free(gradient_y);
        return;
    }
    for (int i = 0; i < width * height; i++)
    {
        input_int[i] = (int) input[i];
    }

    // Apply Sobel X and Y convolution
    convolution(input_int, height, width, gradient_x, kernel_x, 3, 1);
    convolution(input_int, height, width, gradient_y, kernel_y, 3, 1);

    free(input_int);

    // Compute gradient magnitude and angle
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int gx = gradient_x[x + y * width];
            int gy = gradient_y[x + y * width];

            if (threshold == -1) {
                magnitude[x + y * width] = (uint8_t) clamp(sqrt(gx * gx + gy * gy), 0, 255);
            } else {
                magnitude[x + y * width] = sqrt(gx * gx + gy * gy) > threshold ? 255 : 0;
            }
            if (angle != NULL)
            {
                angle[x + y * width] = atan2(gy, gx);
            }
        }
    }

    free(gradient_x);
    free(gradient_y);
}