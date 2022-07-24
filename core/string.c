#include <soul/math/macros.h>
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

void string_set_chars(struct string *string, const char *chars)
{
    free(string->chars);

    int length = strlen(chars);

    string->chars   = malloc(length + 1);
    string->length  = length;

    strcpy(string->chars, chars);
}

bool_t string_eq(struct string a, struct string b)
{
    if (a.length != b.length)
        return FALSE;

    return memcmp(a.chars, b.chars, a.length) == 0;
}

bool_t string_eq_ptr(const char *a, const char *b)
{
    return strcmp(a, b) == 0;
}