#ifndef FILE_H
#define FILE_H

char *  file_to_buffer(const char *path, size_t *bytes);
void    file_free_buffer(char *buffer);

#endif // FILE_H