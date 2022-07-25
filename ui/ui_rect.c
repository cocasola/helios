#include <soul/ui/ui_rect.h>

void deserialize_rect(struct json_array *array, struct ui_rect *destination)
{
    int *cursor = (int *)destination;

    list_for_each (struct json_number *, p_number, array->elements) {
        *cursor = (*p_number)->integer;
        ++cursor;
    }
}