#ifndef UI_MARGINS_H
#define UI_MARGINS_H

#include "ui_rect.h"

struct ui_margins
{
    float top;
    float right;
    float left;
    float bottom;
};

#define ui_margins(top, right, left, bottom) ((ui_margins){ top, right, left, bottom })

static inline void ui_margins_subtract(struct ui_rect *rect, struct ui_margins *margins)
{
    rect->position.x += margins->left;
    rect->position.y += margins->top;
    rect->size.x -= margins->right;
    rect->size.y -= margins->bottom;
}

#endif // UI_MARGINS_H