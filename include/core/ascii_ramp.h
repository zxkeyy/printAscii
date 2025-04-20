#ifndef ASCII_RAMP_H
#define ASCII_RAMP_H

#include <stdbool.h>

// Maximum size of a single UTF-8 character (4 bytes)
#define MAX_UTF8_CHAR_SIZE 5  // 4 bytes + null terminator

typedef struct {
    const char* characters;   // UTF-8 encoded character string
    int length;               // Number of characters (not bytes)
} AsciiRamp;

bool ascii_ramp_validate(const AsciiRamp* ramp);
int ascii_ramp_char_length(const char* utf8_str, int index);
int ascii_ramp_total_chars(const char* utf8_str);

#endif // ASCII_RAMP_H