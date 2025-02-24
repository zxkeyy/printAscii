#ifndef NON_MAXIMUM_SUPPRESSION_H
#define NON_MAXIMUM_SUPPRESSION_H

#include "../core/image.h"

void non_maximum_suppression(const uint8_t* magnitude, const float* angle, uint8_t* output, int width, int height);

#endif // NON_MAXIMUM_SUPPRESSION_H