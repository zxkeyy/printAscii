#include <stdbool.h>
#include <string.h>
#include "core/ascii_ramp.h"

bool ascii_ramp_validate(const AsciiRamp* ramp) {
    return ramp && ramp->characters && ramp->length > 0;
}

// Calculate the byte length of a UTF-8 character at a given index
int ascii_ramp_char_length(const char* utf8_str, int index) {
    if (!utf8_str) return 0;
    
    // Navigate to the start of the indexed character
    int char_count = 0;
    int byte_pos = 0;
    
    while (utf8_str[byte_pos] && char_count < index) {
        // Check first byte to determine character length
        unsigned char c = (unsigned char)utf8_str[byte_pos];
        
        if ((c & 0x80) == 0) {
            // ASCII character (0xxxxxxx)
            byte_pos += 1;
        } else if ((c & 0xE0) == 0xC0) {
            // 2-byte character (110xxxxx)
            byte_pos += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // 3-byte character (1110xxxx)
            byte_pos += 3;
        } else if ((c & 0xF8) == 0xF0) {
            // 4-byte character (11110xxx)
            byte_pos += 4;
        } else {
            // Invalid UTF-8 encoding, treat as 1 byte
            byte_pos += 1;
        }
        
        char_count++;
    }
    
    // Now at the start of the requested character, determine its length
    if (!utf8_str[byte_pos]) return 0; // End of string
    
    unsigned char c = (unsigned char)utf8_str[byte_pos];
    
    if ((c & 0x80) == 0) return 1;        // ASCII
    else if ((c & 0xE0) == 0xC0) return 2; // 2-byte
    else if ((c & 0xF0) == 0xE0) return 3; // 3-byte
    else if ((c & 0xF8) == 0xF0) return 4; // 4-byte
    
    return 1; // Invalid UTF-8, treat as 1 byte
}

// Count total number of characters in a UTF-8 string
int ascii_ramp_total_chars(const char* utf8_str) {
    if (!utf8_str) return 0;
    
    int count = 0;
    int pos = 0;
    
    while (utf8_str[pos]) {
        unsigned char c = (unsigned char)utf8_str[pos];
        
        if ((c & 0x80) == 0) {
            // ASCII character (0xxxxxxx)
            pos += 1;
        } else if ((c & 0xE0) == 0xC0) {
            // 2-byte character (110xxxxx)
            pos += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // 3-byte character (1110xxxx)
            pos += 3;
        } else if ((c & 0xF8) == 0xF0) {
            // 4-byte character (11110xxx)
            pos += 4;
        } else {
            // Invalid UTF-8 encoding, treat as 1 byte
            pos += 1;
        }
        
        count++;
    }
    
    return count;
}