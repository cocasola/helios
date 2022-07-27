#include <soul/math/vector.h>

void deserialize_vec4f(struct json_array *array, struct vec4f *destination, void *data)
{
    float *cursor = (float *)destination;

    list_for_each (struct json_number *, p_number, array->elements) {
        *cursor = (*p_number)->decimal;
        ++cursor;
    }
}