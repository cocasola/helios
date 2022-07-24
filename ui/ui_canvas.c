#include <soul/ui/ui_canvas.h>
#include <soul/ui/ui_text.h>

static struct ui_render_cache *create_render_cache(struct soul_instance *soul_instance,
                                                   struct component_descriptor *descriptor)
{
    struct mesh_service *mesh_service = resource_get(soul_instance, MESH_SERVICE);
    struct shader_service *shader_service = resource_get(soul_instance, SHADER_SERVICE);

    struct ui_render_cache *render_cache = resource_create(
        soul_instance,
        "ui_render_cache",
        sizeof(struct ui_render_cache),
        0
    );

    render_cache->canvas_instances      = &descriptor->passive_storage;
    render_cache->shader                = shader_service->defaults.ui;
    render_cache->quad                  = mesh_service->primitives.ui_quad;
    render_cache->colour_uniform        = shader_get_uniform(render_cache->shader, "colour");
    render_cache->is_text_uniform       = shader_get_uniform(render_cache->shader, "is_text");
    render_cache->use_texture_uniform   = shader_get_uniform(render_cache->shader, "use_texture");
    render_cache->matrix_uniform        = shader_get_uniform(render_cache->shader, "matrix");

    return render_cache;
}

static void render_container(struct ui_container *container,
                             struct window *window,
                             struct ui_render_cache *render_cache)
{
    if (container->visible)
        ui_container_draw(container, render_cache, window);

    list_for_each (struct ui_container *, p_child, container->children) {
        render_container(*p_child, window, render_cache);
    }
}

static void render(struct ui_render_cache *render_cache)
{
    shader_bind(render_cache->shader);

    list_for_each (struct ui_canvas, canvas, *render_cache->canvas_instances) {
        window_bind(canvas->window);

        if (canvas->root_container)
            render_container(canvas->root_container, canvas->window, render_cache);
    }
}

void ui_canvas_register_component(struct soul_instance *soul_instance)
{
    struct ecs_service *ecs_service = resource_get(soul_instance, ECS_SERVICE);

    struct component_registry_info registry_info = {
        .name                   = UI_CANVAS,
        .passive_storage_size   = sizeof(struct ui_canvas)
    };

    struct component_descriptor *descriptor = component_register(ecs_service, &registry_info);

    struct ui_render_cache *render_cache = create_render_cache(soul_instance, descriptor);

    ordered_callbacks_insert(
        &soul_instance->callbacks,
        (ordered_callback_t)&render,
        EXECUTION_ORDER_RENDER,
        render_cache,
        FALSE
    );
}

static void on_left_click(struct window *window, struct ui_canvas *canvas)
{
    ui_container_test_mouse(canvas->root_container, &window->input.mouse);
}

static void on_window_resize(struct window *window, struct ui_canvas *canvas)
{
    ui_container_set_rect(canvas->root_container, ui_rect(0, 0, window->width, window->height));
}

void ui_canvas_set_window(struct ui_canvas *canvas, struct window *window)
{
    if (canvas->window) {
        list_remove(&canvas->window->input.on_left_click, canvas->on_left_click_handle);
        list_remove(&canvas->window->on_resize, canvas->on_window_resize_handle);
    }

    canvas->window = window;

    canvas->on_left_click_handle = callbacks_insert(
        &canvas->window->input.on_left_click,
        (callback_t)&on_left_click,
        canvas
    );

    canvas->on_window_resize_handle = callbacks_insert(
        &canvas->window->on_resize,
        (callback_t)&on_window_resize,
        canvas
    );
}