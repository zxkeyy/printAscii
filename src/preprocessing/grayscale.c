#include <stdio.h>
#include <stdlib.h>
#include <core/image.h>
#include <preprocessing/grayscale.h>

uint8_t RGB_blend(uint8_t r, uint8_t g, uint8_t b) {
    return (uint8_t)((r * 77 + g * 153 + b * 26) >> 8);
}

uint8_t RGBA_blend(uint8_t r, uint8_t g, uint8_t b, uint8_t a, uint8_t alpha_value) {
    const uint8_t rgb = RGB_blend(r, g, b);
    return (uint8_t)(((255 - a) * alpha_value + a * rgb) / 255);
}

bool RGB_image_to_grayscale(Image* img) {
    if (!img) {
        fprintf(stderr, "Image is NULL\n");
        return false;
    }

    if(img->type != IMAGE_TYPE_RGB) {
        fprintf(stderr, "Image is not RGB\n");
        return false;
    }

    int total_pixels = img->width * img->height;
    uint8_t* src = img->pixels;
    uint8_t* dst = img->pixels; // Process in-place to bypass malloc/free bottlenecks completely!
    
    int i = 0;
    int limit = total_pixels - 3;
    // Loop unrolling to process 4 pixels per iteration
    for (; i < limit; i += 4) {
        dst[0] = (src[0]*77 + src[1]*153 + src[2]*26) >> 8;
        dst[1] = (src[3]*77 + src[4]*153 + src[5]*26) >> 8;
        dst[2] = (src[6]*77 + src[7]*153 + src[8]*26) >> 8;
        dst[3] = (src[9]*77 + src[10]*153 + src[11]*26) >> 8;
        src += 12;
        dst += 4;
    }
    for (; i < total_pixels; i++) {
        *dst++ = (src[0]*77 + src[1]*153 + src[2]*26) >> 8;
        src += 3;
    }

    img->type = IMAGE_TYPE_GRAY;
    img->channels = 1;
    return true;
}

bool RGBA_image_to_grayscale(Image* img, uint8_t alpha_value) {
    if (!img) {
        fprintf(stderr, "Image is NULL\n");
        return false;
    }

    if(img->type != IMAGE_TYPE_RGBA) {
        fprintf(stderr, "Image is not RGBA\n");
        return false;
    }

    int total_pixels = img->width * img->height;
    uint8_t* src = img->pixels;
    uint8_t* dst = img->pixels; // In-place
    
    for (int i = 0; i < total_pixels; i++) {
        uint8_t a = src[3];
        uint32_t rgb = (src[0]*77 + src[1]*153 + src[2]*26) >> 8;
        uint32_t val = (255 - a) * alpha_value + a * rgb;
        *dst++ = (val * 257) >> 16; // Fast integer approximation for / 255
        src += 4;
    }

    img->type = IMAGE_TYPE_GRAY;
    img->channels = 1;
    return true;
}

bool image_to_grayscale(Image* img, uint8_t alpha_value) {
    if (!img) {
        fprintf(stderr, "Image is NULL\n");
        return false;
    }

    switch(img->type) {
        case IMAGE_TYPE_RGB:
            return RGB_image_to_grayscale(img);
        case IMAGE_TYPE_RGBA:
            return RGBA_image_to_grayscale(img, alpha_value);
        case IMAGE_TYPE_GRAY:
            return true;
        default:
            fprintf(stderr, "Unsupported image type\n");
            return false;
    }
}