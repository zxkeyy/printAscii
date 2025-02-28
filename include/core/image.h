#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    IMAGE_TYPE_GRAY,    // 8-bit grayscale
    IMAGE_TYPE_RGB,     // 24-bit color
    IMAGE_TYPE_RGBA     // 32-bit color with alpha
} ImageType;

typedef struct {
    uint8_t* pixels;    // Pixel data buffer
    int width;          // Image width in pixels
    int height;         // Image height in pixels
    int channels;       // Number of color channels
    ImageType type;     // Image type
} Image;

typedef struct {
    uint8_t r, g, b;
} RGBColor;

typedef struct {
    uint8_t r, g, b, a;
} RGBAColor;

// Creation/destruction
Image* image_create(int width, int height, ImageType type);
Image* image_clone(const Image* src);
void image_free(Image* img);

// Validation
bool image_validate(const Image* img);
bool image_compare(const Image* a, const Image* b);

// Pixel access
uint8_t* image_pixel_at(const Image* img, int x, int y);
RGBColor get_rgb_color(const Image* img, int x, int y, RGBColor* background_color); // Background color is only used for RGBA images with alpha < 255
RGBAColor get_rgba_color(const Image* img, int x, int y);

#endif // IMAGE_H