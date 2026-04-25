#include "io/video_loader.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/image.h"

static char* shell_escape_single_quotes(const char* input) {
    if (!input) {
        return NULL;
    }

    size_t input_len = strlen(input);
    size_t extra = 0;
    for (size_t i = 0; i < input_len; i++) {
        if (input[i] == '\'') {
            extra += 3;
        }
    }

    size_t escaped_len = input_len + extra + 1;
    char* escaped = malloc(escaped_len);
    if (!escaped) {
        return NULL;
    }

    size_t write_idx = 0;
    for (size_t i = 0; i < input_len; i++) {
        if (input[i] == '\'') {
            escaped[write_idx++] = '\'';
            escaped[write_idx++] = '\\';
            escaped[write_idx++] = '\'';
            escaped[write_idx++] = '\'';
        } else {
            escaped[write_idx++] = input[i];
        }
    }

    escaped[write_idx] = '\0';
    return escaped;
}

static float parse_fps_text(const char* text) {
    if (!text || text[0] == '\0') {
        return 0.0f;
    }

    char* slash = strchr(text, '/');
    if (slash) {
        char numerator_buf[64] = {0};
        char denominator_buf[64] = {0};
        size_t numerator_len = (size_t)(slash - text);
        size_t denominator_len = strlen(slash + 1);

        if (numerator_len >= sizeof(numerator_buf) || denominator_len >= sizeof(denominator_buf)) {
            return 0.0f;
        }

        memcpy(numerator_buf, text, numerator_len);
        memcpy(denominator_buf, slash + 1, denominator_len);

        char* endptr = NULL;
        errno = 0;
        double numerator = strtod(numerator_buf, &endptr);
        if (errno != 0 || !endptr || *endptr != '\0') {
            return 0.0f;
        }

        errno = 0;
        double denominator = strtod(denominator_buf, &endptr);
        if (errno != 0 || !endptr || *endptr != '\0' || denominator == 0.0) {
            return 0.0f;
        }

        return (float)(numerator / denominator);
    }

    char* endptr = NULL;
    errno = 0;
    double fps = strtod(text, &endptr);
    if (errno != 0 || !endptr || *endptr != '\0') {
        return 0.0f;
    }

    return (float)fps;
}

static int probe_video_info(const char* input_path, int* out_width, int* out_height, float* out_fps) {
    char* escaped_path = shell_escape_single_quotes(input_path);
    if (!escaped_path) {
        fprintf(stderr, "Failed to allocate escaped input path\n");
        return -1;
    }

    char command[4096];
    int written = snprintf(
        command,
        sizeof(command),
        "ffprobe -v error -select_streams v:0 -show_entries stream=width,height,r_frame_rate -of default=noprint_wrappers=1:nokey=1 '%s'",
        escaped_path
    );
    free(escaped_path);

    if (written < 0 || (size_t)written >= sizeof(command)) {
        fprintf(stderr, "Failed to build ffprobe command\n");
        return -1;
    }

    FILE* probe_pipe = popen(command, "r");
    if (!probe_pipe) {
        perror("Failed to run ffprobe");
        return -1;
    }

    char width_buf[64] = {0};
    char height_buf[64] = {0};
    char fps_buf[64] = {0};

    if (!fgets(width_buf, sizeof(width_buf), probe_pipe) ||
        !fgets(height_buf, sizeof(height_buf), probe_pipe) ||
        !fgets(fps_buf, sizeof(fps_buf), probe_pipe)) {
        pclose(probe_pipe);
        fprintf(stderr, "Failed to read video metadata from ffprobe\n");
        return -1;
    }

    pclose(probe_pipe);

    width_buf[strcspn(width_buf, "\r\n")] = '\0';
    height_buf[strcspn(height_buf, "\r\n")] = '\0';
    fps_buf[strcspn(fps_buf, "\r\n")] = '\0';

    int width = atoi(width_buf);
    int height = atoi(height_buf);
    float fps = parse_fps_text(fps_buf);

    if (width <= 0 || height <= 0) {
        fprintf(stderr, "Invalid video dimensions from ffprobe: %dx%d\n", width, height);
        return -1;
    }

    if (fps <= 0.0f) {
        fps = 24.0f;
    }

    *out_width = width;
    *out_height = height;
    *out_fps = fps;
    return 0;
}

int video_stream_open(VideoStream* stream, const char* input_path, float fps_override) {
    if (!stream || !input_path) {
        return -1;
    }

    memset(stream, 0, sizeof(*stream));

    int width = 0;
    int height = 0;
    float fps = 0.0f;
    if (probe_video_info(input_path, &width, &height, &fps) != 0) {
        return -1;
    }

    if (fps_override > 0.0f) {
        fps = fps_override;
    }

    char* escaped_path = shell_escape_single_quotes(input_path);
    if (!escaped_path) {
        fprintf(stderr, "Failed to allocate escaped input path\n");
        return -1;
    }

    char command[4096];
    int written = snprintf(
        command,
        sizeof(command),
        "ffmpeg -hide_banner -loglevel fatal -nostats -i '%s' -f rawvideo -pix_fmt rgb24 -vsync 0 -",
        escaped_path
    );
    free(escaped_path);

    if (written < 0 || (size_t)written >= sizeof(command)) {
        fprintf(stderr, "Failed to build ffmpeg command\n");
        return -1;
    }

    FILE* frame_pipe = popen(command, "r");
    if (!frame_pipe) {
        perror("Failed to run ffmpeg");
        return -1;
    }

    size_t frame_size = (size_t)width * (size_t)height * 3u;
    if (frame_size == 0) {
        pclose(frame_pipe);
        fprintf(stderr, "Invalid frame size computed for video stream\n");
        return -1;
    }

    stream->pipe = frame_pipe;
    stream->width = width;
    stream->height = height;
    stream->fps = fps;
    stream->frame_size = frame_size;
    stream->frame_index = 0;

    return 0;
}

int video_stream_read_frame(VideoStream* stream, Image** out_image) {
    if (!stream || !stream->pipe || !out_image) {
        return -1;
    }

    *out_image = NULL;

    Image* image = image_create(stream->width, stream->height, IMAGE_TYPE_RGB);
    if (!image) {
        return -1;
    }

    size_t read_bytes = fread(image->pixels, 1, stream->frame_size, stream->pipe);
    if (read_bytes == stream->frame_size) {
        stream->frame_index++;
        *out_image = image;
        return 1;
    }

    image_free(image);

    if (read_bytes == 0 && feof(stream->pipe)) {
        return 0;
    }

    if (feof(stream->pipe)) {
        return 0;
    }

    if (ferror(stream->pipe)) {
        perror("Failed to read video frame data");
    }

    return -1;
}

void video_stream_close(VideoStream* stream) {
    if (!stream) {
        return;
    }

    if (stream->pipe) {
        pclose(stream->pipe);
        stream->pipe = NULL;
    }
}
