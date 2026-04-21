#ifndef INVERT_H
#define INVERT_H

#include <stdbool.h>
#include "../core/image.h"

bool invert_grayscale_image(Image* img);
bool invert_rgb_image(Image* img);
bool invert_rgba_image(Image* img);
bool invert_image(Image* img);

#endif // INVERT_H