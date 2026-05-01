#ifndef CAST_EXPORTER_H
#define CAST_EXPORTER_H

typedef struct CastExporter CastExporter;

/**
 * Initializes a new CastExporter to write an Asciinema v2 format file.
 * 
 * @param path The path to the output .cast file.
 * @param width The nominal width of the terminal.
 * @param height The nominal height of the terminal.
 * @return A pointed to a new CastExporter, or NULL on error.
 */
CastExporter* cast_exporter_init(const char* path, int width, int height);

/**
 * Writes a frame to the export file.
 * 
 * @param exporter The exporter context.
 * @param time_offset The time in seconds since the start of recording.
 * @param data The raw string payload (e.g. ANSI escape sequences and text) to write.
 * @return 0 on success, < 0 on failure.
 */
int cast_exporter_write_frame(CastExporter* exporter, double time_offset, const char* data);

/**
 * Closes the export file and frees the exporter context.
 * 
 * @param exporter The exporter context.
 */
void cast_exporter_close(CastExporter* exporter);

#endif // CAST_EXPORTER_H
