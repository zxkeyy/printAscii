#include "benchmark.h"
#include "core/image.h"
#include "conversion/image_to_halfblock.h"
#include "preprocessing/grayscale.h"
#include <string.h>

// --- SETUP FUNCTIONS --- //
void* setup_1080p_rgb() {
    Image* img = malloc(sizeof(Image));
    if (!img) return NULL;
    img->width = 1920;
    img->height = 1080;
    img->channels = 3;
    img->type = IMAGE_TYPE_RGB;
    img->pixels = malloc(img->width * img->height * img->channels);
    
    // Fill with gradient data to prevent extreme compiler optimizations 
    // wiping out constant branches
    for(int i = 0; i < img->width * img->height * img->channels; i++) {
        img->pixels[i] = i % 256;
    }
    return img;
}

void teardown_image(void* state) {
    Image* img = (Image*)state;
    if (img) {
        free(img->pixels);
        free(img);
    }
}

// --- RUN/EXECUTION FUNCTIONS --- //
void run_halfblock(void* state) {
    Image* img = (Image*)state;
    char* result = image_to_halfblock(img);
    if (result) {
        (void)result[0];
    }
}

void run_rgb_to_grayscale(void* state) {
    Image* original = (Image*)state;
    
    // Since our grayscale method operates in-place, we must copy the 
    // original dummy data over to a fresh Image struct per iteration
    Image test_img = *original;
    test_img.pixels = malloc(original->width * original->height * 3);
    memcpy(test_img.pixels, original->pixels, original->width * original->height * 3);
    
    RGB_image_to_grayscale(&test_img);
    free(test_img.pixels);
}

int main() {
    printf("--- PrintAscii Benchmarks ---\n\n");

    // Benchmark 1
    Benchmark bench1 = {
        .name = "image_to_halfblock (1920x1080 RGB)",
        .setup = setup_1080p_rgb,
        .run = run_halfblock,
        .teardown = teardown_image,
        .iterations = 100 // Lower iterations since string building is heavy
    };
    run_benchmark(&bench1);

    return 0;
}
