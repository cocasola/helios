#ifndef UI_CANVAS_H
#define UI_CANVAS_H

#include "../core.h"
#include "../ecs.h"
#include "../callbacks.h"
#include "../graphics/mesh.h"
#include "../graphics/shader.h"
#include "window.h"
#include "ui_container.h"

#define UI_CANVAS "ui_canvas"

struct ui_canvas
{
    struct window *         window;
    struct ui_container *   root_container;
    struct callback *       on_left_click_handle;
    struct callback *       on_window_resize_handle;
};

void ui_canvas_register_component(struct soul_instance *soul_instance);
void ui_canvas_set_window(struct ui_canvas *canvas, struct window *window);

#endif // UI_CANVAS_H