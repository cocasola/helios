#include <soul/ui/ui_canvas.h>

static void cleanup(struct entity *entity, struct component_storage storage, void *data) { }

void ui_canvas_register_component(struct soul_instance *soul_instance)
{
    struct ecs_service *ecs_service = resource_get(soul_instance, ECS_SERVICE);

    struct component_registry_info registry_info = {
        .name                   = UI_CANVAS,
        .passive_storage_size   = sizeof(struct ui_canvas),
        .cleanup                = (component_callback_t)&cleanup,
    };

    component_register(ecs_service, &registry_info);
}