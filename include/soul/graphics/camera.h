#ifndef CAMERA_H
#define CAMERA_H

#include "../ecs.h"
#include "render_target.h"

#define CAMERA "camera"

struct camera
{
    COMPONENT

    struct render_target    render_target;
    float                   size;
};

void camera_register_component(struct soul_instance *soul_instance);
void camera_set_target_window(struct camera *camera, struct window *window);
void camera_bind(struct camera *camera);

#endif // CAMERA_H