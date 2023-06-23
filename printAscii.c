#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int countDistance(char *num1,char *num2);
char *getBinaryPixelGrid(int a, int b, int x, int y, unsigned char *imageData, int imageWidth);
char findAsciiMatch(char *pixelGrid, unsigned char *imageData, char *valuesList);
void printAsciiImage(int width, int height, unsigned char *imageData, char *valuesList, int x, int y);
void printAsciiImageToFile(int width, int height, unsigned char *imageData, char *valuesList, int x, int y, char *output);

void convertAlphaToWhite(int width,int height,int channels, unsigned char *imageData);
void inverseColors(int width,int height,int channels, unsigned char *imageData);

int threshold = 128;
int alpha = 255;

int main(int argc, char *argv[]){
    //Parsing command line input
    if(argc < 2){
        fprintf(stderr, "Usage: %s [-i / -t 0<n<255 / -a] [image file] [AsciiGridDict.txt]\n", argv[0]);
        return __LINE__;
    }

    bool inverseColorsB = false;
    bool alphaToWhite = false;
    bool output = false;
    char outputFile[255];

    size_t optind;
    for (optind = 1; optind < argc && argv[optind][0] == '-'; optind++) {
        switch (argv[optind][1]) {
        case 'i': inverseColorsB = true; break;
        case 'a': 
            alphaToWhite = true; 
            optind++;
            alpha = atoi(argv[optind]);
            if (alpha > 255 || alpha < 0){
                fprintf(stderr, "Usage: %s [-i / -t 0<n<255 / -a 0<n<255 / -o [output.txt]] [image file] [AsciiGridDict.txt]\n", argv[0]);
                return __LINE__;
            }
            break;
        case 't':
            optind++;
            threshold = atoi(argv[optind]);
            if (threshold > 255 || threshold < 0){
                fprintf(stderr, "Usage: %s [-i / -t 0<n<255 / -a 0<n<255 / -o [output.txt]] [image file] [AsciiGridDict.txt]\n", argv[0]);
                return __LINE__;
            }
            
            break;
        case 'o':
            optind++;
            output = true;
            strcpy(outputFile, argv[optind]);
            break;
        default:
            fprintf(stderr, "Usage: %s [-i / -t 0<n<255 / -a 0<n<255 / -o [output.txt]] [image file] [AsciiGridDict.txt]\n", argv[0]);
            return __LINE__;
        }   
    }
    argv += optind;


    //Opening the image as 2 channel matrix
    int width, height, channels;
    unsigned char *imageData = stbi_load(argv[0], &width, &height, &channels, 2);
    channels = 2;

    if (imageData == NULL) {
        fprintf(stderr, "Failed to load the image file: %s.\n", argv[0]);
        return __LINE__;
    }
    /*
    printf("Image width: %d\n", width);
    printf("Image height: %d\n", height);
    printf("Image channels: %d\n", channels);
    */
    if(alphaToWhite){
        convertAlphaToWhite(width, height, channels, imageData);
    }
    if(inverseColorsB)
    {
        inverseColors(width, height, channels, imageData);
    }
    if(output == false){
        printAsciiImage(width, height, imageData, argv[1], 7, 15);
    }else{
        printAsciiImageToFile(width, height, imageData, argv[1], 7, 15, outputFile);
    }


    stbi_image_free(imageData);
    return 0;
}


int countDistance(char *num1,char *num2){
    int distance = 0, len = strlen(num1);

    if (len != strlen(num2))
    {
        fprintf(stderr,"length mismatch for comparison\n");
        return -1;
    }

    for (int i = 0; i < len; i++)
    {
        if(num1[i] != num2[i]){
            distance++;
        }
    }
    return distance;
}

char gridBuffer[255] = "";
char *getBinaryPixelGrid(int a, int b, int x, int y, unsigned char *imageData, int imageWidth){
    strcpy(gridBuffer, "");

    for (int k = b; k < y; k++)
    {
        for (int i = a*2; i < x*2; i=i+2)
        {
            if (imageData[i + imageWidth*2*k] > threshold)
            {
                strcat(gridBuffer, "1");
            }else{
                strcat(gridBuffer, "0");
            }
        }
    }

    return gridBuffer;
}

char findAsciiMatch(char *pixelGrid, unsigned char *imageData, char *valuesList){
    //Opening file with Ascii characteres and thier 2D grid
    FILE *fp = fopen(valuesList, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening the file %s\n", valuesList);
        exit(EXIT_FAILURE);
    }

    int closestDistance = 255, distance;
    char closestChar = ' ';
    char lineBuffer[255];

    //Reading the file line by line
    while (fgets(lineBuffer, 200, fp))
    {
        lineBuffer[strcspn(lineBuffer, "\n")] = 0; //removes \n at the end of the line

        //Finding closest match
        distance = countDistance(lineBuffer+2, pixelGrid);
        if(distance == -1){
            fclose(fp);
            exit(EXIT_FAILURE);
        }
        if (distance < closestDistance)
        {
            closestDistance = distance;
            closestChar = lineBuffer[0];
        }
    }
 
    fclose(fp);
    return closestChar;
}

void printAsciiImage(int width, int height, unsigned char *imageData, char *valuesList, int x, int y){
    for (int i = y; i < height; i=i+y)
    {
        for (int k = x; k < width; k=k+x)
        {
            printf("%c", findAsciiMatch(getBinaryPixelGrid(k-x, i-y, k, i, imageData, width), imageData, valuesList));
        } 
        printf("\n");
    }
}

void printAsciiImageToFile(int width, int height, unsigned char *imageData, char *valuesList, int x, int y, char *output){
    FILE *fp = fopen(output, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening the file %s\n", output);
        exit(EXIT_FAILURE);
    }

    for (int i = y; i < height; i=i+y)
    {
        for (int k = x; k < width; k=k+x)
        {
            fprintf(fp, "%c", findAsciiMatch(getBinaryPixelGrid(k-x, i-y, k, i, imageData, width), imageData, valuesList));
        } 
        fprintf(fp ,"\n");
    }
    fclose(fp);
}

void convertAlphaToWhite(int width,int height,int channels, unsigned char *imageData){
    for (int i = channels-1; i < width * height * channels; i=i+channels)
    {
        if (imageData[i] == 0)
        {
            for (int k = 0; k < channels; k++)
            {
                imageData[i-k] = alpha;
            }
        }
    }
}

void inverseColors(int width,int height,int channels, unsigned char *imageData){
    for (int i = 0; i < width * height * channels; i=i+channels)
    {
        imageData[i] = 255 - imageData[i];
    }
}