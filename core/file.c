#include <stdio.h>
#include <stdlib.h>

#include <soul/file.h>

char *file_to_buffer(const char *path, size_t *bytes)
{
    FILE *file = fopen(path, "r");

    if (!file)
        return 0;

    fseek(file, 0, SEEK_END);
    size_t file_length = ftell(file);
    rewind(file);

    char *buffer = (char *)calloc(1, file_length + 1);

    for (int i = 0;; ++i) {
        int c = fgetc(file);
        if (c == EOF)
            break;

        buffer[i] = c;
    }

    if (bytes)
        *bytes = file_length;

    fclose(file);

    return buffer;
}

void file_free_buffer(char *buffer)
{
    free(buffer);
}