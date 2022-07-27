#ifndef JSON_H
#define JSON_H

#include "string.h"
#include "string_map.h"
#include "list.h"

typedef int json_element_t;
#define JSON_NUMBER 0
#define JSON_STRING 1
#define JSON_OBJECT 2
#define JSON_ARRAY  3

typedef int json_number_t;
#define JSON_NUMBER_INTEGER 0
#define JSON_NUMBER_DECIMAL 1

#define JSON_ELEMENT    json_element_t          type;   \
                        struct string           name;   \
                        struct json_element *   parent;

struct json_element
{
    JSON_ELEMENT
};

struct json_number
{
    JSON_ELEMENT

    json_number_t number_type;

    union {
        int     integer;
        float   decimal;
    };
};

struct json_string
{
    JSON_ELEMENT

    struct string string;
};

struct json_array
{
    JSON_ELEMENT

    struct list elements; // json_element *
};

struct json_object
{
    JSON_ELEMENT

    struct string_map children; // json_element *
};

struct json_object *json_parse_string(const char *json);
void                json_destroy_object(struct json_object *object);
void                json_print_object(struct json_object *object);
void *              json_index_object(struct json_object *object, const char *name);

#endif // JSON_H