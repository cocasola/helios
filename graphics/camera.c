#include <soul/graphics/camera.h>

static void init(struct entity *entity, struct component_storage storage, void *data)
{
    struct camera *const camera = storage.active;

    camera->size        = 1.0;
    camera->transform   = entity->transform;
}

void camera_register_component(struct soul_instance *soul_instance)
{
    struct ecs_service *ecs_service = resource_get(soul_instance, ECS_SERVICE);

    struct component_registry_info registry_info = {
        .name                   = CAMERA,
        .active_storage_size    = sizeof(struct camera),
        .init                   = (component_callback_t)&init,
    };

    component_register(ecs_service, &registry_info);
}

void camera_bind(struct camera *camera)
{
    render_target_bind(camera->render_target);
}