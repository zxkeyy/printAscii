#include <stdio.h>
#include <stdlib.h>
#include "utilities/hysteresis_edge_track.h"

void hysteresis_edge_track(Image* img){
    if (img == NULL) {
        fprintf(stderr, "Image is NULL\n");
        return;
    }
    if(img->type != IMAGE_TYPE_GRAY) {
        fprintf(stderr, "Image is not grayscale\n");
        return;
    }

    // Allocate memory for the output image
    Image* output = image_create(img->width, img->height, IMAGE_TYPE_GRAY);
    if (!output) {
        fprintf(stderr, "Failed to allocate output image\n");
        return;
    }

    // Buffer acts as queue to store x,y pairs to store pixel cords
    int* queue = calloc(2* img->height * img->width, sizeof(int));
    int head = 0;
    int tail = 0;
    
    for(int y = 0; y < img->height; y++) {
        for(int x = 0; x < img->width; x++) {
            uint8_t* pixel = image_pixel_at(img, x, y);
            if (*pixel == 255) // Strong edge
            {
                // Copy pixel to output
                pixel = image_pixel_at(output, x, y);
                *pixel = 255;

                // Add to queue
                queue[tail] = x;
                queue[tail + 1] = y;
                tail += 2;
            }
        }
    }

    int x,y;
    while (head != tail){
        // Pop from queue
        x = queue[head];
        y = queue[head + 1];
        head += 2;

        // Loop over neghboring pixels to find connected weak edges
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                uint8_t* pixel = image_pixel_at(img, x + i, y + j);
                if (*pixel == 128) //Weak edge
                {
                    // Mark as strong edge
                    *pixel = 255;

                    // // Add pixel to output
                    pixel = image_pixel_at(output, x + i, y + j);
                    *pixel = 255;

                    // Add to queue
                    queue[tail] = x + i;
                    queue[tail + 1] = y + j;
                    tail += 2;
                }
            }
        }
    }

    free(queue);

    free(img->pixels);
    img->pixels = output->pixels;
    output->pixels = NULL;
    image_free(output);
}