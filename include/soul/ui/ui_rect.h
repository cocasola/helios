#ifndef UI_RECT_H
#define UI_RECT_H

struct ui_rect;

#include "../math/vector.h"
#include "../json.h"
#include "ui_alignment.h"
#include "ui_axis.h"

struct ui_rect
{
    struct vec2i position;
    struct vec2i size;
};

#define ui_rect(px, py, sx, sy) ((struct ui_rect){ vec2i(px, py), vec2i(sx, sy) })

static inline void ui_rect_align(struct ui_rect *rect, ui_alignment_t alignment, ui_axis_t axis)
{
    switch (alignment) {
        case UI_ALIGNMENT_CENTER:
            ui_axis_p(rect->position, axis) -= ui_axis_p(rect->size, axis)/2;
            break;
    }
}

void deserialize_rect(struct json_array *array, struct ui_rect *destination);

#endif // UI_RECT_H