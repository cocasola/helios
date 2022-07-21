#ifndef UI_VIEWPORT_H
#define UI_VIEWPORT_H

#include "../core.h"
#include "../graphics/texture.h"

#define UI_VIEWPORT "ui_viewport"

struct ui_viewport
{
    struct render_target *  render_target;
    struct ui_container *   container;
};

void ui_viewport_register_component(struct soul_instance *soul_instance);

#endif // UI_VIEWPORT_H