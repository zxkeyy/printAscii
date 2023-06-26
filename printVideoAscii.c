#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
   FILE *fp = fopen(argv[1], "r");
   if (fp == NULL)
   {
      fprintf(stderr, "Error opening the file %s\n", argv[1]);
      return -1;
   }

   float framerate = ((float) 1 )/((float) atoi(argv[2])) * 1000000;
   char buffer[500];
   while(fgets(buffer, 500, fp)) {
      if(buffer[0] == '\n'){
         usleep(framerate);
         system("clear");
         fgets(buffer, 500, fp);
      }else{
         printf("%s", buffer);
      }

   }

   fclose(fp);
   return 0;
}