#ifndef CAMERA_H
#define CAMERA_H

#include "../ecs.h"
#include "texture.h"

#define CAMERA "camera"

struct camera
{
    struct transform *      transform;
    struct render_target *  render_target;
    float                   size;
};

void camera_register_component(struct soul_instance *soul_instance);
void camera_bind(struct camera *camera);

#endif // CAMERA_H