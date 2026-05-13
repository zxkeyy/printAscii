#ifndef CANNY_EDGE_DETECTION_H
#define CANNY_EDGE_DETECTION_H

#include <stdbool.h>
#include "core/image.h"

bool canny_edge_detection(Image* img, float sigma, int high_threshold, int low_threshold,
						  int auto_sigma, int auto_high, int auto_low);

#endif // CANNY_EDGE_DETECTION_H