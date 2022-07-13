#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

#include "../ui/window.h"

typedef int render_target_mode_t;
#define RENDER_TARGET_WINDOW    0
#define RENDER_TARGET_VIEWPORT  1

struct render_target;

typedef void(*render_target_binder_t)(struct render_target *render_target);

struct render_target
{
    render_target_mode_t    mode;
    void *                  target;
    render_target_binder_t  binder;
    int                     width;
    int                     height;
};

void render_target_set_window(struct render_target *render_target, struct window *window);

static inline void render_target_bind(struct render_target *render_target)
{
    render_target->binder(render_target);
}

#endif // RENDER_TARGET_H