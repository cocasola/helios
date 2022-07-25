#ifndef PROPERTY_SERIALIZATION_H
#define PROPERTY_SERIALIZATION_H

#include "json.h"

typedef void(*property_deserializer_t)(struct json_element *json, void *destination);

struct property_serializer
{
    property_deserializer_t deserializer;
};

void property_serialization_populate_table(struct string_map *serializers);

#endif // PROPERTY_SERIALIZATION_H