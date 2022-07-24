#include <soul/ui/ui_render.h>

struct mat4x4 ui_render_calculate_matrix(struct ui_rect *rect, int depth, struct window *window)
{
    struct mat4x4 r = MAT4X4_IDENTITY;

    float z = -depth/(float)65535;

    mat4x4_set_pos(
        &r,
        vec3f(
            rect->position.x,
            -rect->position.y,
            z
        )
    );

    mat4x4_set_scale(
        &r,
        vec3f(rect->size.x, rect->size.y, 1)
    );

    struct mat4x4 view_matrix = MAT4X4_IDENTITY;

    mat4x4_set_pos(
        &view_matrix,
        vec3f(
            -1,
            1,
            0
        )
    );

    mat4x4_set_scale(
        &view_matrix,
        vec3f(
            1.0/window->width*2.0,
            1.0/window->height*2.0,
            1.0
        )
    );

    r = mul4x4(&view_matrix, &r);

    return r;
}
