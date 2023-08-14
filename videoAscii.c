#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

    char *outputFile = argv[1];
    char *video = argv[2];
    int frames = atoi(argv[3]);

    char imagesFolder[] = "tempFFMPEG";
    char command[200];
    
    sprintf(command, "mkdir %s", imagesFolder);
    system(command);
    sprintf(command, "cd %s", imagesFolder);
    system(command);
    sprintf(command, "ffmpeg -i \"%s\" %s/%%d.png",video, imagesFolder);
    system(command);
    printf("\nProcessing video...");
    sprintf(command, "echo \"\" > %s", outputFile);
    system(command);

    for (int i = 1; i <= frames; i++)
    {
        sprintf(command, "./printAscii -m 1 %s/%d.png AsciiBi2.txt >> %s", imagesFolder, i, outputFile);
        system(command);
        sprintf(command, "echo '\n' >> %s", outputFile);
        system(command);
    }

    sprintf(command, "rm -r %s", imagesFolder);
    system(command);
    
    printf("\nDone. You can play the video using: ./printVideoAscii %s %d\n", outputFile, frames);

    return 0;
}
