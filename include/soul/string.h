#ifndef STRING_H
#define STRING_H

#include <stdlib.h>
#include <string.h>

#include "typedefs.h"

struct string
{
    char *  chars;
    size_t  length;
};

struct string   string_create(const char *string);
void            string_destroy(struct string string);
void            string_set_chars(struct string *string, const char *chars);
bool_t          string_eq_ptr(const char *a, const char *b);

#endif // STRING_H