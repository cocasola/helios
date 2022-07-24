#ifndef UI_RENDER_CACHE_H
#define UI_RENDER_CACHE_H

#include "../list.h"
#include "../ui/window.h"
#include "../graphics/shader.h"
#include "../graphics/mesh.h"
#include "ui_rect.h"

struct ui_render_cache
{
    struct list *   canvas_instances; // struct ui_canvas
    struct mesh *   quad;
    struct shader * shader;
    uniform_t       matrix_uniform;
    uniform_t       colour_uniform;
    uniform_t       use_texture_uniform;
    uniform_t       is_text_uniform;
};

struct mat4x4 ui_render_calculate_matrix(struct ui_rect *rect, int depth, struct window *window);

#endif // UI_RENDER_CACHE_H