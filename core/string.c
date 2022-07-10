#include <soul/string.h>

struct string string_create(const char *string)
{
    size_t length = strlen(string);

    char *chars = (char *)malloc(length + 1);
    strcpy(chars, string);

    return (struct string){ chars, length };
}

void string_destroy(struct string string)
{
    free(string.chars);
}

bool_t string_eq_ptr(const char *a, const char *b)
{
    return strcmp(a, b) == 0;
}
