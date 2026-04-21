#ifndef SOBEL_EDGE_DETECTION_H
#define SOBEL_EDGE_DETECTION_H

#include <stdbool.h>
#include "../core/image.h"

bool sobel_edge_detection(Image* img, uint16_t threshold);

#endif // SOBEL_EDGE_DETECTION_H