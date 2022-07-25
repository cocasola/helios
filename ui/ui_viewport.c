#include <soul/ecs.h>
#include <soul/callbacks.h>
#include <soul/ui/ui_viewport.h>
#include <soul/ui/ui_container.h>

#include <soul/graphics/core.h>

struct callback_data
{
    struct ecs_service *    ecs;
    struct texture_service *texture_service;
};

static void on_resize(struct ui_container *container, struct ui_viewport *viewport)
{
    render_target_resize(
        viewport->render_target,
        container->absolute_rect.size.x,
        container->absolute_rect.size.y
    );
}

static void init(struct entity *entity,
                 struct component_storage storage,
                 struct callback_data *data)
{
    struct ui_viewport *const viewport = storage.passive;

    viewport->container = component_get_storage(data->ecs, entity, UI_CONTAINER).passive;
    callbacks_insert(&viewport->container->on_resize, (callback_t)&on_resize, viewport);

    int width = viewport->container->absolute_rect.size.x;
    if (!width)
        width = 1;

    int height = viewport->container->absolute_rect.size.y;
    if (!height)
        height = 1;

    struct render_target_create_info render_target_info = NEW_RENDER_TARGET_CREATE_INFO;
    render_target_info.width        = width;
    render_target_info.height       = height;
    render_target_info.filter_mode  = TEXTURE_FILTERMODE_NEAREST;

    viewport->render_target = render_target_create(data->texture_service, &render_target_info);
    viewport->container->texture = viewport->render_target->texture;
}

static void clear(struct list *viewport_instances)
{
    list_for_each (struct ui_viewport, viewport, *viewport_instances) {
        render_target_bind(viewport->render_target);
        graphics_set_clear_colour(vec4f(0.0, 0.0, 0.0, 0.0));
        graphics_clear();
    }
}

void ui_viewport_register_component(struct soul_instance *soul_instance)
{
    struct ecs_service *ecs = resource_get(soul_instance, ECS_SERVICE);

    struct callback_data *callback_data = resource_create(
        soul_instance,
        "ui_viewport_callback_data",
        sizeof(struct callback_data),
        0
    );

    callback_data->ecs              = ecs;
    callback_data->texture_service  = resource_get(soul_instance, TEXTURE_SERVICE);

    struct component_registry_info info = {
        .name                   = UI_VIEWPORT,
        .passive_storage_size   = sizeof(struct ui_viewport),
        .callbacks.init         = (component_callback_t)&init,
        .callbacks.data         = callback_data
    };

    struct component_descriptor *descriptor = component_register(ecs, &info);

    ordered_callbacks_insert(
        &soul_instance->callbacks,
        (ordered_callback_t)&clear,
        EXECUTION_ORDER_PRE_RENDER,
        &descriptor->passive_storage,
        FALSE
    );
}