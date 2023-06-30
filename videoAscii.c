#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

    char command[200];
    char *outputFile = argv[1];
    char *imagesFolder = argv[2];
    int frames = atoi(argv[3]);
    for (int i = 1; i <= frames; i++)
    {
        sprintf(command, "./printAscii -m 1 %s/%d.png AsciiBi2.txt >> %s", imagesFolder, i, outputFile);
        system(command);
        sprintf(command, "echo '\n' >> %s", outputFile);
        system(command);
    }
    
    
    return 0;
}
