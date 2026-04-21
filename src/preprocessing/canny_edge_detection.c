#include <stdio.h>
#include <stdlib.h>
#include "preprocessing/canny_edge_detection.h"
#include "preprocessing/gaussian_blur.h"
#include "utilities/sobel_operator.h"
#include "utilities/non_maximum_suppression.h"
#include "utilities/double_threshold.h"
#include "utilities/hysteresis_edge_track.h"

bool canny_edge_detection(Image* img, float sigma, int high_threshold, int low_threshold){
    if (img == NULL) {
        fprintf(stderr, "Image is NULL\n");
        return false;
    }
    if(img->type != IMAGE_TYPE_GRAY) {
        fprintf(stderr, "Image is not grayscale\n");
        return false;
    }

    gaussian_blur(img, sigma);

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

    // Not needed anymore
    free(angles);
    free(magnitude);

    double_threshold(img, high_threshold, low_threshold);

    hysteresis_edge_track(img);
    return true;
}