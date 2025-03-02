#ifndef IMAGE_TO_ANSI_H
#define IMAGE_TO_ANSI_H

#include "core/image.h"
#include "core/ascii_ramp.h"

char* image_to_ansi(Image* img, char* tiling_string, RGBColor background_color, int background);
char* image_to_alpha_ansi(Image* img, char* tiling_string, AsciiRamp ramp, int background);

#endif // IMAGE_TO_ANSI_H