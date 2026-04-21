#ifndef RESIZE_H
#define RESIZE_H

#include <stdbool.h>
#include "core/image.h"

bool image_resize(Image* img, int new_width, int new_height);

#endif // RESIZE_H