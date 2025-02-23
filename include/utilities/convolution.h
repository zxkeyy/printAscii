#ifndef CONVOLUTION_H
#define CONVOLUTION_H

#include "../core/image.h"

void convolution(const int* input, int input_height, int input_width,
                 int* output, const int* kernel, int kernel_size, int divisor);

#endif // CONVOLUTION_H