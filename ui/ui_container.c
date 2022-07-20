#include <soul/ecs.h>
#include <soul/math/matrix.h>
#include <soul/graphics/shader.h>
#include <soul/graphics/mesh.h>
#include <soul/ui/ui_container.h>
#include <soul/ui/ui_axis.h>

struct callback_data
{
    struct ecs_service *ecs;
};

struct render_cache
{
    struct list *   canvas_instances; // struct ui_canvas
    struct mesh *   quad;
    struct shader * shader;
    uniform_t       matrix_uniform;
    uniform_t       colour_uniform;
    uniform_t       use_texture_uniform;
    uniform_t       is_text_uniform;
};

static void init(struct entity *entity, struct component_storage storage, void *data)
{
    struct ui_container *const container = storage.passive;

    container->entity = entity;

    container->type = UI_TYPE_CONTAINER;
    container->draw_axis = ui_axis_get_layout_axis(container->layout);

    list_init(&container->children, sizeof(struct ui_element *));
    list_init(&container->on_click, sizeof(struct callback));
    list_init(&container->on_resize, sizeof(struct callback));
    list_init(&container->on_move, sizeof(struct callback));
}

static void entered_tree(struct entity *entity,
                         struct component_storage storage,
                         struct callback_data *data)
{
    struct ui_container *const container = storage.passive;

    ui_element_link_ancestry((struct ui_element *)container, entity, data->ecs);

    struct ui_canvas *parent_canvas = component_get_storage(
        data->ecs,
        entity->parent,
        UI_CANVAS
    ).passive;

    if (parent_canvas) {
        parent_canvas->root_container = container;
        ui_container_calculate(container);
    }
}

static void cleanup(struct entity *entity, struct component_storage storage, void *data)
{
    struct ui_container *const container = storage.passive;

    list_destroy(&container->children);
    list_destroy(&container->on_click);
    list_destroy(&container->on_resize);
    list_destroy(&container->on_move);
}

static struct mat4x4 calculate_matrix(struct ui_container *container,
                                      struct window *window)
{
    struct mat4x4 r = MAT4X4_IDENTITY;

    float z = container->depth/(float)65535;

    mat4x4_set_pos(
        &r,
        vec3f(
            container->absolute_rect.position.x,
            -container->absolute_rect.position.y,
            z
        )
    );

    mat4x4_set_scale(
        &r,
        vec3f(container->absolute_rect.size.x, container->absolute_rect.size.y, 1)
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

static void render_container(struct ui_container *container,
                             struct window *window,
                             struct render_cache *render_cache)
{
    if (container->visible) {
        struct mat4x4 matrix = calculate_matrix(container, window);

        shader_uniform_int(render_cache->use_texture_uniform, FALSE);
        shader_uniform_vec4f(render_cache->colour_uniform, container->colour);
        shader_uniform_mat4x4(render_cache->matrix_uniform, &matrix);

        mesh_draw(render_cache->quad);
    }

    list_for_each (struct ui_element *, p_child, container->children) {
        if ((*p_child)->type == UI_TYPE_CONTAINER)
            render_container((struct ui_container *)(*p_child), window, render_cache);
    }
}

static void render(struct render_cache *render_cache)
{
    shader_bind(render_cache->shader);
    shader_uniform_int(render_cache->is_text_uniform, FALSE);

    list_for_each (struct ui_canvas, canvas, *render_cache->canvas_instances) {
        window_bind(canvas->window);

        if (canvas->root_container)
            render_container(canvas->root_container, canvas->window, render_cache);
    }
}

static struct render_cache *create_render_cache(struct soul_instance *soul_instance)
{
    struct ecs_service *ecs = resource_get(soul_instance, ECS_SERVICE);
    struct mesh_service *mesh_service = resource_get(soul_instance, MESH_SERVICE);
    struct shader_service *shader_service = resource_get(soul_instance, SHADER_SERVICE);

    struct render_cache *render_cache = resource_create(
        soul_instance,
        "ui_container_render_cache",
        sizeof(struct render_cache),
        0
    );

    struct component_descriptor *ui_canvas_descriptor = component_match_descriptor(ecs, UI_CANVAS);

    render_cache->canvas_instances      = &ui_canvas_descriptor->passive_storage;
    render_cache->shader                = shader_service->defaults.ui;
    render_cache->quad                  = mesh_service->primitives.ui_quad;
    render_cache->colour_uniform        = shader_get_uniform(render_cache->shader, "colour");
    render_cache->is_text_uniform       = shader_get_uniform(render_cache->shader, "is_text");
    render_cache->use_texture_uniform   = shader_get_uniform(render_cache->shader, "use_texture");
    render_cache->matrix_uniform        = shader_get_uniform(render_cache->shader, "matrix");

    return render_cache;
}

void ui_container_register_component(struct soul_instance *soul_instance)
{
    struct ecs_service *ecs_service = resource_get(soul_instance, ECS_SERVICE);

    struct callback_data *callback_data = resource_create(
        soul_instance,
        "ui_container_callback_data",
        sizeof(struct callback_data),
        0
    );

    callback_data->ecs = resource_get(soul_instance, ECS_SERVICE);

    struct component_registry_info registry_info = {
        .name                   = UI_CONTAINER,
        .passive_storage_size   = sizeof(struct ui_container),
        .init                   = (component_callback_t)&init,
        .entered_tree           = (component_callback_t)&entered_tree,
        .cleanup                = (component_callback_t)&cleanup,
        .callback_data          = callback_data
    };

    component_register(ecs_service, &registry_info);

    struct render_cache *render_cache = create_render_cache(soul_instance);

    ordered_callbacks_insert(
        &soul_instance->callbacks,
        (ordered_callback_t)&render,
        EXECUTION_ORDER_RENDER,
        render_cache,
        FALSE
    );
}

void ui_container_set_rect(struct ui_container *container, struct ui_rect rect)
{
    container->rect = rect;

    if (rect.size.x > 0)
        container->absolute_rect.size.x = rect.size.x;
    if (rect.size.y > 0)
        container->absolute_rect.size.y = rect.size.y;

    ui_container_calculate(container);

    /*
     * ui_container_calculate() will automatically dispatch on_resize for elements resized,
     * and this only occurs when a rect has a less than or equal to 0 size component.
     */
    if (rect.size.x > 0 && rect.size.y > 0)
        callbacks_dispatch(&container->on_resize, container);
}

static struct ui_container *calculate_minimums(struct ui_container *container)
{
    return 0;
}

static struct vec2i calculate_ui_max_size(struct ui_container *container)
{
    if (container->draw_axis == -1)
        return VEC2I_ZERO;

    int max_sizes = 0;
    int size_total = 0;

    int available_pixels = ui_axis(container->absolute_rect.size, container->draw_axis);

    list_for_each (struct ui_element *, p_child, container->children) {
        switch ((*p_child)->type) {
            case UI_TYPE_CONTAINER: ;
                struct ui_container *const child_container = (struct ui_container *)(*p_child);

                if (ui_axis(child_container->rect.size, container->draw_axis) == UI_MAX_SIZE) {
                    ++max_sizes;
                } else {
                    int size = ui_axis(child_container->absolute_rect.size, container->draw_axis);

                    available_pixels -= size;
                    size_total += size;
                }
                break;
        }
    }

    struct vec2i max_size = container->absolute_rect.size;

    if (max_sizes && available_pixels) {
        ui_axis(max_size, container->draw_axis) = available_pixels/max_sizes;
        container->children_size = size_total + available_pixels;
    }

    return max_size;
}

static void calculate_sizes(struct ui_container *container)
{
    struct vec2i max_size = calculate_ui_max_size(container);

    list_for_each (struct ui_element *, p_child, container->children) {
        switch ((*p_child)->type) {
            case UI_TYPE_CONTAINER: ;
                struct ui_container *child = (struct ui_container *)(*p_child);

                bool_t resized = FALSE;

                if (child->rect.size.x == UI_MAX_SIZE) {
                    child->absolute_rect.size.x = max_size.x;
                    resized = TRUE;
                }

                if (child->rect.size.y == UI_MAX_SIZE) {
                    child->absolute_rect.size.y = max_size.y;
                    resized = TRUE;
                }

                if (resized)
                    callbacks_dispatch(&child->on_resize, child);

                calculate_sizes(child);
                break;
        }
    }
}

static struct vec2i init_cursor(struct ui_container *container)
{
    struct ui_rect rect = container->absolute_rect;
    ui_margins_subtract(&rect, &container->margins);

    struct vec2i cursor;

    switch (container->alignment) {
        case UI_ALIGNMENT_TOP_LEFT: ;
            cursor = vec2i(rect.position.x, rect.position.y);
            break;

        case UI_ALIGNMENT_CENTER: ;
            int pos = ui_axis(rect.position, container->draw_axis);
            int pos_p = ui_axis_p(rect.position, container->draw_axis);

            int size = ui_axis(rect.size, container->draw_axis);
            int size_p = ui_axis(rect.size, container->draw_axis);

            int center_p = pos_p + size_p/2;
            int center = pos + size/2;

            ui_axis_p(cursor, container->draw_axis) = center_p;
            ui_axis(cursor, container->draw_axis) = center - container->children_size/2;
            break;
    }

    return cursor;
}

static void calculate_positions_linear(struct ui_container *container)
{
    struct vec2i cursor = init_cursor(container);

    list_for_each (struct ui_element *, p_child, container->children) {
        int cursor_offset = 0;

        switch ((*p_child)->type) {
            case UI_TYPE_CONTAINER: ;
                struct ui_container *const container = (struct ui_container *)(*p_child);

                container->absolute_rect.position = cursor;

                ui_rect_align(
                    &container->absolute_rect,
                    container->alignment,
                    container->draw_axis
                );

                cursor_offset = ui_axis(container->absolute_rect.size, container->draw_axis);
                break;
        }

        ui_axis(cursor, container->draw_axis) += cursor_offset;
    }
}

static void calculate_positions(struct ui_container *container, int depth)
{
    if (container->draw_axis != -1 && container->children.head)
        calculate_positions_linear(container);

    list_for_each (struct ui_element *, p_child, container->children) {
        (*p_child)->depth = depth + 1;

        if ((*p_child)->type == UI_TYPE_CONTAINER)
            calculate_positions((struct ui_container *)(*p_child), depth + 1);
    }
}

void ui_container_calculate(struct ui_container *container)
{
    struct ui_container *most_shallow_modified = calculate_minimums(container);
    if (!most_shallow_modified)
        most_shallow_modified = container;

    if (most_shallow_modified->parent) {
        calculate_sizes(most_shallow_modified->parent);
        calculate_positions(most_shallow_modified->parent, container->depth);
    } else {
        calculate_sizes(most_shallow_modified);
        calculate_positions(most_shallow_modified, container->depth);
    }
}

void ui_container_set_layout(struct ui_container *container, ui_layout_t layout)
{
    container->layout = layout;
    container->draw_axis = ui_axis_get_layout_axis(container->layout);
    ui_container_calculate(container);
}

void ui_container_set_alignment(struct ui_container *container, ui_alignment_t alignment)
{
    container->alignment = alignment;
    calculate_positions(container, container->depth);
}