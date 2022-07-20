#ifndef UI_CANVAS_H
#define UI_CANVAS_H

#include "../core.h"
#include "../ecs.h"
#include "window.h"
#include "ui_container.h"

#define UI_CANVAS "ui_canvas"

struct ui_canvas
{
    struct window *         window;
    struct ui_container *   root_container;
};

void ui_canvas_register_component(struct soul_instance *soul_instance);

#endif // UI_CANVAS_H