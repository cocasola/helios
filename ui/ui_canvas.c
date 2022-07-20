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

static void on_left_click(struct window *window, struct ui_canvas *canvas)
{
    ui_container_test_mouse(canvas->root_container, &window->input.mouse);
}

void ui_canvas_set_window(struct ui_canvas *canvas, struct window *window)
{
    if (canvas->window)
        list_remove(&canvas->window->input.on_left_click, canvas->on_left_click_handle);

    canvas->window = window;

    canvas->on_left_click_handle = callbacks_subscribe(
        &canvas->window->input.on_left_click,
        (callback_t)&on_left_click,
        canvas
    );
}