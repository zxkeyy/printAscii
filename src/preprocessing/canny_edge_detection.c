#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "preprocessing/canny_edge_detection.h"
#include "preprocessing/gaussian_blur.h"
#include "utilities/sobel_operator.h"
#include "utilities/non_maximum_suppression.h"
#include "utilities/double_threshold.h"
#include "utilities/hysteresis_edge_track.h"
#include "utilities/clamp.h"

static float clampf(float value, float min, float max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

static int percentile_from_hist(const int* hist, int total, float percentile) {
    if (total <= 0) {
        return 0;
    }

    int target = (int)ceilf((float)total * percentile);
    int cumulative = 0;
    for (int i = 0; i < 256; i++) {
        cumulative += hist[i];
        if (cumulative >= target) {
            return i;
        }
    }

    return 255;
}

static float estimate_sigma_from_laplacian(const Image* img) {
    const int width = img->width;
    const int height = img->height;
    int histogram[256] = {0};

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int x0 = clamp(x - 1, 0, width - 1);
            int x1 = clamp(x + 1, 0, width - 1);
            int y0 = clamp(y - 1, 0, height - 1);
            int y1 = clamp(y + 1, 0, height - 1);

            int center = img->pixels[y * width + x];
            int left = img->pixels[y * width + x0];
            int right = img->pixels[y * width + x1];
            int up = img->pixels[y0 * width + x];
            int down = img->pixels[y1 * width + x];
            int lap = abs((4 * center) - left - right - up - down);

            if (lap > 255) {
                lap = 255;
            }
            histogram[lap] += 1;
        }
    }

    int total = width * height;
    int median = percentile_from_hist(histogram, total, 0.5f);
    float noise = (float)median / 255.0f;

    return clampf(0.6f + (1.5f * noise), 0.6f, 2.0f);
}

bool canny_edge_detection(Image* img, float sigma, int high_threshold, int low_threshold,
                          int auto_sigma, int auto_high, int auto_low) {
    if (img == NULL) {
        fprintf(stderr, "Image is NULL\n");
        return false;
    }
    if(img->type != IMAGE_TYPE_GRAY) {
        fprintf(stderr, "Image is not grayscale\n");
        return false;
    }

    float effective_sigma = sigma;
    if (auto_sigma) {
        effective_sigma = estimate_sigma_from_laplacian(img);
    }

    gaussian_blur(img, effective_sigma);

    // Allocate memory for sobel outputs
    float* angles = malloc(img->width * img->height * sizeof(float));
    uint8_t* magnitude = malloc(img->width * img->height * sizeof(uint8_t));
    if (!angles || !magnitude) {
        fprintf(stderr, "Failed to allocate Canny intermediate buffers\n");
        free(angles);
        free(magnitude);
        return false;
    }

    // Apply sobel operator
    sobel_operator(img->pixels, magnitude, angles, img->width, img->height, -1);

    non_maximum_suppression(magnitude, angles, img->pixels, img->width, img->height);

    int effective_high = high_threshold;
    int effective_low = low_threshold;
    if (auto_high || auto_low) {
        int histogram[256] = {0};
        int total = img->width * img->height;
        for (int i = 0; i < total; i++) {
            histogram[img->pixels[i]] += 1;
        }

        if (auto_high) {
            effective_high = percentile_from_hist(histogram, total, 0.92f);
            if (effective_high < 1) {
                effective_high = 1;
            }
        }

        if (auto_low) {
            int base_high = auto_high ? effective_high : high_threshold;
            int computed_low = (int)((float)base_high * 0.4f + 0.5f);
            effective_low = clamp(computed_low, 0, 255);
        }

        if (effective_low >= effective_high) {
            if (auto_high && !auto_low) {
                int adjusted_high = effective_low + 1;
                effective_high = adjusted_high <= 255 ? adjusted_high : 255;
            } else if (auto_low) {
                effective_low = effective_high > 1 ? (effective_high - 1) : 0;
            }
        }
    }

    // Not needed anymore
    free(angles);
    free(magnitude);

    double_threshold(img, (uint8_t)effective_high, (uint8_t)effective_low);

    hysteresis_edge_track(img);
    return true;
}