#include "core/image.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Image* image_create(int width, int height, ImageType type) {
    if (width <= 0 || height <= 0) {
        fprintf(stderr, "Invalid image dimensions: %dx%d\n", width, height);
        return NULL;
    }

    Image* img = malloc(sizeof(Image));
    if (!img) {
        perror("Failed to allocate image structure");
        return NULL;
    }

    img->width = width;
    img->height = height;
    img->type = type;

    switch (type) {
        case IMAGE_TYPE_GRAY:
            img->channels = 1;
            break;
        case IMAGE_TYPE_RGB:
            img->channels = 3;
            break;
        case IMAGE_TYPE_RGBA:
            img->channels = 4;
            break;
        default:
            fprintf(stderr, "Invalid image type: %d\n", type);
            free(img);
            return NULL;
    }

    const size_t buffer_size = width * height * img->channels;
    img->pixels = calloc(buffer_size, sizeof(uint8_t)); // Calloc to avoid image ghosting
    
    if (!img->pixels) {
        perror("Failed to allocate pixel buffer");
        free(img);
        return NULL;
    }

    return img;
}

Image* image_clone(const Image* src) {
    if (!image_validate(src)) {
        fprintf(stderr, "Cannot clone invalid image\n");
        return NULL;
    }

    Image* copy = image_create(src->width, src->height, src->type);
    if (!copy) return NULL;

    const size_t buffer_size = src->width * src->height * src->channels;
    memcpy(copy->pixels, src->pixels, buffer_size);
    
    return copy;
}

void image_free(Image* img) {
    if (img) {
        free(img->pixels);
        free(img);
    }
}

bool image_validate(const Image* img) {
    if (!img) return false;
    if (img->width <= 0 || img->height <= 0) return false;
    if (!img->pixels) return false;
    
    const int expected_channels = (img->type == IMAGE_TYPE_GRAY) ? 1 :
                                 (img->type == IMAGE_TYPE_RGB) ? 3 :
                                 (img->type == IMAGE_TYPE_RGBA) ? 4 : 0;
    
    return img->channels == expected_channels;
}

bool image_compare(const Image* a, const Image* b) {
    if (!image_validate(a) || !image_validate(b)) return false;
    if (a->width != b->width || a->height != b->height) return false;
    if (a->type != b->type || a->channels != b->channels) return false;

    if (a->pixels == b->pixels) return true; // If both point to the same buffer

    const size_t buffer_size = a->width * a->height * a->channels;
    return memcmp(a->pixels, b->pixels, buffer_size) == 0;
}

uint8_t* image_pixel_at(const Image* img, int x, int y) {
    if (!image_validate(img)) return NULL;
    if (x < 0 || x >= img->width || y < 0 || y >= img->height) return NULL;
    
    return &img->pixels[(y * img->width + x) * img->channels];
}

RGBColor get_rgb_color(const Image* img, int x, int y, RGBColor* background_color){
    const uint8_t* pixel = image_pixel_at(img, x, y);
    RGBColor color = {0, 0, 0};
    
    switch(img->type) {
        case IMAGE_TYPE_GRAY:
            color.r = color.g = color.b = pixel[0];
            break;
        case IMAGE_TYPE_RGB:
            color.r = pixel[0];
            color.g = pixel[1];
            color.b = pixel[2];
            break;
        case IMAGE_TYPE_RGBA:
            // Alpha blending calculation
            uint8_t alpha = pixel[3];
            color.r = (pixel[0] * alpha / 255) + (background_color->r * (255 - alpha) / 255);
            color.g = (pixel[1] * alpha / 255) + (background_color->g * (255 - alpha) / 255);
            color.b = (pixel[2] * alpha / 255) + (background_color->b * (255 - alpha) / 255);
            break;
    }
    
    return color;
}

RGBAColor get_rgba_color(const Image* img, int x, int y){
    const uint8_t* pixel = image_pixel_at(img, x, y);
    RGBAColor color = {0, 0, 0, 0};
    
    switch(img->type) {
        case IMAGE_TYPE_GRAY:
            color.r = color.g = color.b = pixel[0];
            color.a = 255;
            break;
        case IMAGE_TYPE_RGB:
            color.r = pixel[0];
            color.g = pixel[1];
            color.b = pixel[2];
            color.a = 255;
            break;
        case IMAGE_TYPE_RGBA:
            color.r = pixel[0];
            color.g = pixel[1];
            color.b = pixel[2];
            color.a = pixel[3];
            break;
    }
    
    return color;
}