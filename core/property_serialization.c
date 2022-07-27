#include <soul/property_serialization.h>
#include <soul/math/vector.h>
#include <soul/ui/ui_rect.h>
#include <soul/ui/font.h>
#include <soul/ui/ui_margins.h>
#include <soul/graphics/texture.h>

static void deserialize_int(struct json_number *number, int *destination, void *data)
{
    *destination = number->integer;
}

static void deserialize_float(struct json_number *number, float *destination, void *data)
{
    *destination = number->decimal;
}

static void deserialize_string(struct json_string *string, struct string *destination, void *data)
{
    *destination = string_create(string->string.chars);
}

static void register_serializer(struct string_map *serializers,
                                const char *name,
                                property_deserializer_t deserializer,
                                void *data)
{
    struct property_serializer serializer_struct = {
        deserializer,
        data
    };

    string_map_insert(
        serializers,
        name,
        &serializer_struct
    );
}

void property_serialization_populate_table(struct soul_instance *soul_instance,
                                           struct string_map *serializers)
{
    struct texture_service *texture_service = resource_get(soul_instance, TEXTURE_SERVICE);
    struct font_service *font_service = resource_get(soul_instance, FONT_SERVICE);

    register_serializer(serializers, "int", (property_deserializer_t)&deserialize_int, 0);
    register_serializer(serializers, "float", (property_deserializer_t)&deserialize_float, 0);
    register_serializer(serializers, "string", (property_deserializer_t)&deserialize_string, 0);
    register_serializer(serializers, "vec4f", (property_deserializer_t)&deserialize_vec4f, 0);
    register_serializer(serializers, "rect", (property_deserializer_t)&deserialize_rect, 0);
    register_serializer(serializers, "margins", (property_deserializer_t)&deserialize_margins, 0);

    register_serializer(
        serializers,
        "font",
        (property_deserializer_t)&deserialize_font,
        font_service
    );

    register_serializer(
        serializers,
        "texture",
        (property_deserializer_t)&deserialize_texture,
        texture_service
    );
}