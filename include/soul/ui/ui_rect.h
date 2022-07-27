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

static inline struct vec2i ui_rect_calculate_cursor(struct ui_rect *rect,
                                                    ui_alignment_t alignment,
                                                    int draw_size,
                                                    ui_axis_t axis)
{
    struct vec2i cursor = rect->position;

    switch (alignment) {
        case UI_ALIGNMENT_CENTER:
            cursor.x += rect->size.x/2;
            cursor.y += rect->size.y/2;

            UI_AXIS(cursor, axis) -= draw_size/2;
            break;

        case UI_ALIGNMENT_LEFT:
            cursor.y += rect->size.y/2;

            if (axis == VEC2I_YOFFSET)
                cursor.y -= draw_size/2;
            break;
    }

    return cursor;
}

static inline void ui_rect_align(struct ui_rect *rect, ui_alignment_t alignment, ui_axis_t axis)
{
    switch (alignment) {
        case UI_ALIGNMENT_CENTER:
            UI_AXIS_P(rect->position, axis) -= UI_AXIS_P(rect->size, axis)/2;
            break;
    }
}

void deserialize_rect(struct json_array *array, struct ui_rect *destination, void *data);

#endif // UI_RECT_H