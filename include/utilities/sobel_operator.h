#ifndef SOBEL_OPERATOR_H
#define SOBEL_OPERATOR_H

#include "../core/image.h"

void sobel_operator(const uint8_t* input, uint8_t* magnitude, float* angle, int width, int height, int threshold);

#endif // SOBEL_OPERATOR_H