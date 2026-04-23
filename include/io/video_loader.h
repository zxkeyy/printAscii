#ifndef VIDEO_LOADER_H
#define VIDEO_LOADER_H

#include <stddef.h>
#include <stdio.h>

#include "core/image.h"

typedef struct {
    FILE* pipe;
    int width;
    int height;
    float fps;
    size_t frame_size;
    int frame_index;
} VideoStream;

int video_stream_open(VideoStream* stream, const char* input_path, float fps_override);
int video_stream_read_frame(VideoStream* stream, Image** out_image);
void video_stream_close(VideoStream* stream);

#endif // VIDEO_LOADER_H
