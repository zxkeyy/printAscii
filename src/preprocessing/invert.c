#include <stdio.h>
#include <stdlib.h>
#include <core/image.h>
#include <preprocessing/invert.h>

bool invert_grayscale_image(Image* img) {
    if (!img) {
        fprintf(stderr, "Image is NULL\n");
        return false;
    }

    if(img->type != IMAGE_TYPE_GRAY) {
        fprintf(stderr, "Image is not grayscale\n");
        return false;
    }

    for(int y = 0; y < img->height; y++) {
        for(int x = 0; x < img->width; x++) {
            uint8_t* pixel = image_pixel_at(img, x, y);
            *pixel = 255 - *pixel;
        }
    }

    return true;
}

bool invert_rgb_image(Image* img) {
    if (!img) {
        fprintf(stderr, "Image is NULL\n");
        return false;
    }

    if(img->type != IMAGE_TYPE_RGB) {
        fprintf(stderr, "Image is not RGB\n");
        return false;
    }

    for(int y = 0; y < img->height; y++) {
        for(int x = 0; x < img->width; x++) {
            uint8_t* pixel = image_pixel_at(img, x, y);
            pixel[0] = 255 - pixel[0];
            pixel[1] = 255 - pixel[1];
            pixel[2] = 255 - pixel[2];
        }
    }

    return true;
}

bool invert_rgba_image(Image* img) {
    if (!img) {
        fprintf(stderr, "Image is NULL\n");
        return false;
    }

    if(img->type != IMAGE_TYPE_RGBA) {
        fprintf(stderr, "Image is not RGBA\n");
        return false;
    }

    for(int y = 0; y < img->height; y++) {
        for(int x = 0; x < img->width; x++) {
            uint8_t* pixel = image_pixel_at(img, x, y);
            pixel[0] = 255 - pixel[0];
            pixel[1] = 255 - pixel[1];
            pixel[2] = 255 - pixel[2];
        }
    }

    return true;
}

bool invert_image(Image* img) {
    if (!img) {
        fprintf(stderr, "Image is NULL\n");
        return false;
    }

    switch(img->type) {
        case IMAGE_TYPE_GRAY:
            return invert_grayscale_image(img);
        case IMAGE_TYPE_RGB:
            return invert_rgb_image(img);
        case IMAGE_TYPE_RGBA:
            return invert_rgba_image(img);
        default:
            fprintf(stderr, "Unsupported image type\n");
            return false;
    }
}