#ifndef CANNY_EDGE_DETECTION_H
#define CANNY_EDGE_DETECTION_H

#include "core/image.h"

void canny_edge_detection(Image* img, float sigma, int high_threshold, int low_threshold);

#endif // CANNY_EDGE_DETECTION_H