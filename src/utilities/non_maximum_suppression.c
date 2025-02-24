#include <math.h>
#include "utilities/non_maximum_suppression.h"
#include "utilities/convolution.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void non_maximum_suppression(const uint8_t* magnitude, const float* angle, uint8_t* output, int width, int height) {
    // Initialize output buffer
    for (int i = 0; i < width * height; i++) {
        output[i] = magnitude[i];
    }

    // Apply non-maximum suppression
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            const int i = x + y * width;
            // Convert angle from radians to degrees and normalize to [0, 180)
            float dir = angle[i] * (180.0f / M_PI);
            if (dir < 0) {
                dir += 180.0f;
            }

            float a = 0, b = 0;
            // Quantize the angle to one of four directions
            if ((dir >= 0 && dir < 22.5f) || (dir >= 157.5f && dir <= 180.0f)) {
                // Horizontal edge: compare left and right
                a = magnitude[i - 1];
                b = magnitude[i + 1];
            } else if (dir >= 22.5f && dir < 67.5f) {
                // 45-degree edge: compare top-right and bottom-left
                a = magnitude[i - width + 1];
                b = magnitude[i + width - 1];
            } else if (dir >= 67.5f && dir < 112.5f) {
                // Vertical edge: compare top and bottom
                a = magnitude[i - width];
                b = magnitude[i + width];
            } else { // (dir >= 112.5f && dir < 157.5f)
                // 135-degree edge: compare top-left and bottom-right
                a = magnitude[i - width - 1];
                b = magnitude[i + width + 1];
            }

            // Suppress the pixel if it is not a local maximum
            if (magnitude[i] < a || magnitude[i] < b) {
                output[i] = 0;
            }
        }
    }

    // Suppress border pixels
    for (int x = 0; x < width; x++) {
        output[x] = 0;
        output[x + (height - 1) * width] = 0;
    }
    for (int y = 0; y < height; y++) {
        output[y * width] = 0;
        output[y * width + width - 1] = 0;
    }
}
