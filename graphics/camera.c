#include <soul/graphics/camera.h>

static void init(struct camera *camera, void *data)
{
    camera->size = 1.0;
}

void camera_register_component(struct soul_instance *soul_instance)
{
    struct ecs_service *ecs_service = resource_get(soul_instance, ECS_SERVICE);

    struct component_registry_info registry_info = {
        .name           = CAMERA,
        .callback_data  = 0,
        .struct_size    = sizeof(struct camera),
        .init           = (component_callback_t)&init,
        .cleanup        = 0,
        .entered_tree   = 0
    };

    component_register(ecs_service, &registry_info);
}

void camera_set_target_window(struct camera *camera, struct window *window)
{
    render_target_set_window(&camera->render_target, window);
}

void camera_bind(struct camera *camera)
{
    render_target_bind(&camera->render_target);
}