#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



int main(int argc,char *argv[]){

    int charWidth = atoi(argv[2]);
    int charHeight = atoi(argv[3]);
    char chars[] = " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

    int width, height, channels;
    unsigned char *imageData = stbi_load(argv[1], &width, &height, &channels, 1);
    channels = 1;
    if (imageData == NULL) {
        fprintf(stderr, "Failed to load the image file: %s.\n", argv[1]);
        return __LINE__;
    }

    for (int i = 0; i <= (width - charWidth)/charWidth; i++)
    {
        printf("%c,", chars[i]);
        for (int k = 0; k < charHeight; k++)
        {
            for (int t = 0; t < charWidth; t++)
            {
                if(imageData[t + k * width + i * charWidth]>=128){
                    printf("1");
                }else{
                    printf("0");
                }
            }
        }
        printf("\n");
    }
    

    stbi_image_free(imageData);
    return 0;
}