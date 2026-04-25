#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>

#ifndef PRINTASCII_BIN_PATH
#define PRINTASCII_BIN_PATH "printAscii"
#endif

static int command_exit_code(const char* command) {
    int status = system(command);
    if (status == -1) {
        return -1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return -1;
}

static long file_size_bytes(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return -1;
    }
    return (long)st.st_size;
}

int main(void) {
    if (command_exit_code("ffmpeg -version > /dev/null 2>&1") != 0) {
        printf("SKIP: ffmpeg not available for integration test\n");
        return 0;
    }

    const char* input_video = "/tmp/printascii_video_integration.mp4";
    const char* out_file = "/tmp/printascii_video_integration.out";
    const char* err_file = "/tmp/printascii_video_integration.err";

    char make_video_cmd[1024];
    snprintf(
        make_video_cmd,
        sizeof(make_video_cmd),
        "ffmpeg -hide_banner -loglevel error -f lavfi -i testsrc=size=80x60:rate=8 -t 1 -y %s",
        input_video
    );

    if (command_exit_code(make_video_cmd) != 0) {
        fprintf(stderr, "Failed to generate integration test video\n");
        return 1;
    }

    char run_cmd[2048];
    snprintf(
        run_cmd,
        sizeof(run_cmd),
        "%s --video --video-max-frames 4 --video-fps 8 -i %s --mode ascii --width 24 > %s 2> %s",
        PRINTASCII_BIN_PATH,
        input_video,
        out_file,
        err_file
    );

    int run_exit = command_exit_code(run_cmd);
    if (run_exit != 0) {
        fprintf(stderr, "Video CLI run failed with exit code %d\n", run_exit);
        return 1;
    }

    long out_size = file_size_bytes(out_file);
    long err_size = file_size_bytes(err_file);

    if (out_size <= 0) {
        fprintf(stderr, "Expected non-empty video CLI output\n");
        return 1;
    }

    if (err_size > 0) {
        fprintf(stderr, "Expected empty stderr for video CLI integration run\n");
        return 1;
    }

    return 0;
}
