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