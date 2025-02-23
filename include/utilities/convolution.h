#ifndef CONVOLUTION_H
#define CONVOLUTION_H

#include "../core/image.h"

void convolution(int* input, int input_height, int input_width, const int* kernel, int kernel_size);

#endif // CONVOLUTION_H