#ifndef UI_AXIS_H
#define UI_AXIS_H

#include <stddef.h>

#include "../math/vector.h"
#include "ui_layout.h"

#define VEC2I_XOFFSET offsetof(struct vec2i, x)
#define VEC2I_YOFFSET offsetof(struct vec2i, y)

#define ui_axis(v, axis) (*(int *)((char *)&(v) + axis))
#define ui_axis_p(v, axis) (*(int *)((char *)&(v) + ((axis == VEC2I_XOFFSET) ? VEC2I_YOFFSET : VEC2I_XOFFSET)))

typedef int ui_axis_t;

static inline ui_axis_t ui_axis_get_layout_axis(ui_layout_t layout)
{
    ui_axis_t axis;

    switch (layout)
    {
    case UI_LAYOUT_HORIZONTAL:
        axis = VEC2I_XOFFSET;
        break;

    case UI_LAYOUT_VERTICAL:
        axis = VEC2I_YOFFSET;
        break;

    default:
        axis = -1;
        break;
    }

    return axis;
}

#endif // UI_AXIS_H