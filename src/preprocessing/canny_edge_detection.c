#include <stdio.h>
#include <stdlib.h>
#include "preprocessing/canny_edge_detection.h"
#include "preprocessing/gaussian_blur.h"
#include "utilities/sobel_operator.h"
#include "utilities/non_maximum_suppression.h"
#include "utilities/double_threshold.h"
#include "utilities/hysteresis_edge_track.h"

void canny_edge_detection(Image* img, float sigma, int high_threshold, int low_threshold){
    if (img == NULL) {
        fprintf(stderr, "Image is NULL\n");
        return;
    }
    if(img->type != IMAGE_TYPE_GRAY) {
        fprintf(stderr, "Image is not grayscale\n");
        return;
    }

    gaussian_blur(img, sigma);

    // Allocate memory for sobel outputs
    float* angles = malloc(img->width * img->height * sizeof(float));
    uint8_t* magnitude = malloc(img->width * img->height * sizeof(uint8_t));

    // Apply sobel operator
    sobel_operator(img->pixels, magnitude, angles, img->width, img->height, -1);

    non_maximum_suppression(magnitude, angles, img->pixels, img->width, img->height);

    // Not needed anymore
    free(angles);
    free(magnitude);

    double_threshold(img, high_threshold, low_threshold);

    hysteresis_edge_track(img);
}