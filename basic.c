#include <stdio.h>
#include <stdlib.h>

char *read_entire_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *result = malloc(file_length);
    size_t bytes_read = fread(result, 1, file_length, file);
    result[bytes_read] = '\0';

    fclose(file);
    return result;
}
