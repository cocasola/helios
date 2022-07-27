#ifndef PROPERTY_SERIALIZATION_H
#define PROPERTY_SERIALIZATION_H

#include "json.h"
#include "core.h"

typedef void(*property_deserializer_t)(struct json_element *json, void *destination, void *data);

struct property_serializer
{
    property_deserializer_t deserializer;
    void *                  data;
};

void property_serialization_populate_table(struct soul_instance *soul_instance,
                                           struct string_map *serializers);

#endif // PROPERTY_SERIALIZATION_H