#ifndef IMAGE_TO_HTML_H
#define IMAGE_TO_HTML_H

#include "core/image.h"
#include "core/ascii_ramp.h"

char* image_to_html(Image* img, char* tiling_string, RGBColor background_color, int background);
char* image_to_alpha_html(Image* img, char* tiling_string, AsciiRamp ramp, int background);

#endif // IMAGE_TO_HTML_H