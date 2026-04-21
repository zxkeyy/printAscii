#ifndef DITHER_H
#define DITHER_H

#include <stdbool.h>
#include "core/image.h"

bool floyd_steinberg_dither(Image* img, uint8_t threshold);

#endif // DITHER_H