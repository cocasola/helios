#ifndef UI_MARGINS_H
#define UI_MARGINS_H

#include "ui_rect.h"

struct ui_margins
{
    int left;
    int top;
    int bottom;
    int right;
};

#define ui_margins(left, top, bottom, right) ((ui_margins){ left, top, bottom, right })

static inline void ui_margins_subtract(struct ui_rect *rect, struct ui_margins *margins)
{
    rect->position.x += margins->left;
    rect->position.y += margins->top;
    rect->size.x -= margins->right + margins->left;
    rect->size.y -= margins->bottom + margins->top;
}

void deserialize_margins(struct json_array *array, struct ui_margins *destination, void *data);

#endif // UI_MARGINS_H