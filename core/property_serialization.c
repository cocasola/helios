#include <soul/property_serialization.h>
#include <soul/math/vector.h>
#include <soul/ui/ui_rect.h>

static void deserialize_int(struct json_number *number, int *destination)
{
    *destination = number->integer;
}

static void deserialize_float(struct json_number *number, float *destination)
{
    *destination = number->decimal;
}

static void deserialize_string(struct json_string *string, struct string *destination)
{
    *destination = string_create(string->string.chars);
}

static void register_serializer(struct string_map *serializers,
                                const char *name,
                                property_deserializer_t deserializer)
{
    struct property_serializer serializer_struct = {
        deserializer
    };

    string_map_insert(
        serializers,
        name,
        &serializer_struct
    );
}

void property_serialization_populate_table(struct string_map *serializers)
{
    register_serializer(serializers, "int", (property_deserializer_t)&deserialize_int);
    register_serializer(serializers, "float", (property_deserializer_t)&deserialize_float);
    register_serializer(serializers, "string", (property_deserializer_t)&deserialize_string);
    register_serializer(serializers, "vec4f", (property_deserializer_t)&deserialize_vec4f);
    register_serializer(serializers, "rect", (property_deserializer_t)&deserialize_rect);
}