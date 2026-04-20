#ifndef DEBUG_ARTIFACTS_H
#define DEBUG_ARTIFACTS_H

#include "core/image.h"
#include "core/config.h"

void save_debug_image(const Image* img, const AppConfig* config, const char* filename);

#endif // DEBUG_ARTIFACTS_H