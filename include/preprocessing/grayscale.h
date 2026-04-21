#ifndef GRAYSCALE_H
#define GRAYSCALE_H

#include <stdbool.h>
#include "../core/image.h"

bool RGB_image_to_grayscale(Image* img);
bool RGBA_image_to_grayscale(Image* img, uint8_t alpha_value);
bool image_to_grayscale(Image* img, uint8_t alpha_value);

#endif // GRAYSCALE_H