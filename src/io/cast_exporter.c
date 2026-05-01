#include "io/cast_exporter.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct CastExporter {
    FILE* file;
    int width;
    int height;
};

// Escapes the payload for JSON. 
// Replaces newlines with "\r\n" appropriately to act like terminal outputs.
static void write_json_string(FILE* out, const char* str) {
    fputc('"', out);
    for (const char* p = str; *p; p++) {
        if (*p == '"') {
            fputs("\\\"", out);
        } else if (*p == '\\') {
            fputs("\\\\", out);
        } else if (*p == '\n') {
            fputs("\\r\\n", out); 
        } else if (*p == '\r') {
            fputs("\\r", out);
        } else if (*p == '\t') {
            fputs("\\t", out);
        } else if (*p > 0 && *p < 0x20) {
            fprintf(out, "\\u%04x", (unsigned char)*p);
        } else {
            fputc(*p, out);
        }
    }
    fputc('"', out);
}

CastExporter* cast_exporter_init(const char* path, int width, int height) {
    if (!path) return NULL;
    
    FILE* f = fopen(path, "w");
    if (!f) return NULL;
    
    CastExporter* exporter = malloc(sizeof(CastExporter));
    if (!exporter) {
        fclose(f);
        return NULL;
    }
    
    exporter->file = f;
    exporter->width = width;
    exporter->height = height;
    
    long timestamp = (long)time(NULL);
    
    // Write Asciinema v2 header
    fprintf(f, "{\"version\": 2, \"width\": %d, \"height\": %d, \"timestamp\": %ld, \"env\": {\"TERM\": \"xterm-256color\"}}\n", 
            width, height, timestamp);
    
    return exporter;
}

int cast_exporter_write_frame(CastExporter* exporter, double time_offset, const char* data) {
    if (!exporter || !exporter->file || !data) return -1;
    
    fprintf(exporter->file, "[%.4f, \"o\", ", time_offset);
    write_json_string(exporter->file, data);
    fprintf(exporter->file, "]\n");
    
    // Ensure data handles crashes smoothly by flushing the file periodically
    fflush(exporter->file);
    return 0;
}

void cast_exporter_close(CastExporter* exporter) {
    if (exporter) {
        if (exporter->file) {
            fclose(exporter->file);
        }
        free(exporter);
    }
}
