#ifndef IMAGE_TO_HALFBLOCK_H
#define IMAGE_TO_HALFBLOCK_H

#include "core/image.h"

/**
 * @brief Converts an image to a string of Unicode half-blocks with ANSI foreground and background colors
 * 
 * @param img The input image to convert
 * @return A dynamically allocated string containing the colored half-blocks, or NULL on failure.
 *         The caller is responsible for freeing the returned string.
 */
char* image_to_halfblock(Image* img);

#endif // IMAGE_TO_HALFBLOCK_H
