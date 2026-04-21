#include <stdio.h>

#include "core/image.h"
#include "preprocessing/dither.h"

#define CLAMP(x) (x < 0 ? 0 : (x > 255 ? 255 : x))

bool floyd_steinberg_dither(Image* img, uint8_t threshold) {
    if (!img) {
        fprintf(stderr, "Image is NULL\n");
        return false;
    }

    if (img->type != IMAGE_TYPE_GRAY) {
        fprintf(stderr, "Image is not grayscale\n");
        return false;
    }

    const int width = img->width;
    const int height = img->height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t* pixel = image_pixel_at(img, x, y);
            const uint8_t old_pixel = pixel[0];
            const uint8_t new_pixel = old_pixel <= threshold ? 0 : 255;
            const int error = old_pixel - new_pixel;
            pixel[0] = new_pixel;

            if (x + 1 < width) {
                pixel = image_pixel_at(img, x + 1, y);
                pixel[0] = CLAMP(pixel[0] + (error * 7 >> 4));
            }
            if (y + 1 < height) {
                pixel = image_pixel_at(img, x, y + 1);
                pixel[0] = CLAMP(pixel[0] + (error * 5 >> 4));
            }
            if (x - 1 >= 0 && y + 1 < height) {
                pixel = image_pixel_at(img, x - 1, y + 1);
                pixel[0] = CLAMP(pixel[0] + (error * 3 >> 4));
            }
            if (x + 1 < width && y + 1 < height) {
                pixel = image_pixel_at(img, x + 1, y + 1);
                pixel[0] = CLAMP(pixel[0] + (error * 1 >> 4));
            }
        }
    }

    return true;
}