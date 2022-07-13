#include <soul/ui/window.h>
#include <soul/graphics/render_target.h>

static void bind_window(struct render_target *render_target)
{
    window_bind((struct window *)render_target->target);
}

void render_target_set_window(struct render_target *render_target, struct window *window)
{
    render_target->target   = window;
    render_target->mode     = RENDER_TARGET_WINDOW;
    render_target->binder   = bind_window;
    render_target->width    = window->width;
    render_target->height   = window->height;
}