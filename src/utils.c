#include <stdio.h>
#include <stdlib.h>

#define CHUNK_SIZE 1024  // Adjust the chunk size as needed

/*
Reads an entire file while dynamically allocating sufficient memory.
Brought to you by ChatGPT.
*/
char* read_entire_house(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening %s\n", filename);
        return NULL;
    }

    char* buffer = NULL;
    size_t buffer_size = 0;
    size_t total_size = 0;

    while (!feof(file)) {
        if (total_size + CHUNK_SIZE > buffer_size) {
            buffer_size += CHUNK_SIZE;
            char* new_buffer = realloc(buffer, buffer_size);
            if (!new_buffer) {
                perror("Error reallocating buffer");
                free(buffer);
                fclose(file);
                return NULL;
            }
            buffer = new_buffer;
        }

        size_t read_size = fread(buffer + total_size, 1, CHUNK_SIZE, file);
        total_size += read_size;
    }

    fclose(file);

    // Resize the buffer to the actual size
    char* result = realloc(buffer, total_size + 1);  // +1 for null terminator
    if (!result) {
        perror("Error reallocating buffer");
        free(buffer);
        return NULL;
    }

    result[total_size] = '\0';  // Null-terminate the string
    return result;
}