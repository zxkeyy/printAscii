#include <stdio.h>
#include <stdint.h>
#include <locale.h>
#include <wchar.h>
#include "utilities/print_utf16_string.h"

void print_utf16_string(int16_t* string){
    setlocale(LC_ALL, ""); // Enable Unicode

#ifdef _WIN32 // I did not test this on windows so idk, chat gpt said it works tho
    wprintf(L"%ls", (wchar_t *)string);
#else
    for (int i = 0; string[i] != 0; i++){
        printf("%lc", (wchar_t)string[i]);
    }
#endif
}

void print_utf16_string_to_file(int16_t* string, const char* filename){
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open output file");
        return;
    }

    setlocale(LC_ALL, ""); // Enable Unicode

    for (int i = 0; string[i] != 0; i++){
        fprintf(file, "%lc", (wchar_t)string[i]);
    }

    fclose(file);
}