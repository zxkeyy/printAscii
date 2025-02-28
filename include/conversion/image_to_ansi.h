#ifndef IMAGE_TO_ANSI_H
#define IMAGE_TO_ANSI_H

#include "../core/image.h"

char* image_to_ansi(Image* img, char* string, RGBColor background_color);

char* grayscale_image_to_ansi(Image* img, char* string);
char* RGB_image_to_ansi(Image* img, char* string);
char* RGBA_image_to_ansi(Image* img, char* string, uint8_t background_brightness);

#endif // IMAGE_TO_ANSI_H