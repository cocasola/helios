#include <soul/ui/ui_margins.h>

void deserialize_margins(struct json_array *array, struct ui_margins *destination, void *data)
{
    int *cursor = (int *)destination;

    list_for_each (struct json_number *, p_number, array->elements) {
        *cursor = (*p_number)->integer;
        ++cursor;
    }
}