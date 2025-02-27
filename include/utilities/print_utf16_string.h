#ifndef PRINT_UTF16_STRING_H
#define PRINT_UTF16_STRING_H

#include <stdint.h>

void print_utf16_string(int16_t* string);
void print_utf16_string_to_file(int16_t* string, const char* filename);

#endif // PRINT_UTF16_STRING_H