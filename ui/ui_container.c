#include <soul/ecs.h>
#include <soul/math/matrix.h>
#include <soul/math/macros.h>
#include <soul/graphics/shader.h>
#include <soul/graphics/mesh.h>
#include <soul/graphics/texture.h>
#include <soul/ui/ui_container.h>
#include <soul/ui/ui_axis.h>
#include <soul/ui/ui_text.h>
#include <soul/ui/ui_canvas.h>

struct callback_data
{
    struct ecs_service *    ecs;
    struct font_service *   font_service;
};

static void init(struct entity *entity,
                 struct component_storage storage,
                 struct callback_data *data)
{
    struct ui_container *const container = storage.passive;

    container->entity       = entity;
    container->draw_axis    = ui_axis_get_layout_axis(container->layout);

    list_init(&container->children, sizeof(struct ui_container *));
    list_init(&container->on_left_click, sizeof(struct callback));
    list_init(&container->on_resize, sizeof(struct callback));
    list_init(&container->on_move, sizeof(struct callback));

    ui_text_init(&container->ui_text);

    if (container->text_font) {
        ui_container_set_text_font(
            data->font_service,
            container,
            container->text_font,
            container->text_size
        );
    }

    if (container->text.chars)
        ui_container_set_text(container, container->text.chars);

    if (container->rect.size.x > 0)
        container->absolute_rect.size.x = container->rect.size.x;

    if (container->rect.size.y > 0)
        container->absolute_rect.size.y = container->rect.size.y;

    if (entity->parent) {
        struct ui_container *parent_container = component_get_storage(
            data->ecs,
            entity->parent,
            UI_CONTAINER
        ).passive;

        if (parent_container) {
            list_push(&parent_container->children, (struct ui_container **)&container);
            container->parent = parent_container;
        }
    }
}

static void entered_tree(struct entity *entity,
                         struct component_storage storage,
                         struct callback_data *data)
{
    struct ui_container *const container = storage.passive;

    if (!entity->parent)
        return;

    struct ui_canvas *parent_canvas = component_get_storage(
        data->ecs,
        entity->parent,
        UI_CANVAS
    ).passive;

    if (parent_canvas) {
        parent_canvas->root_container = container;

        if (parent_canvas->window) {
            ui_container_set_rect(
                container,
                ui_rect(0, 0, parent_canvas->window->width, parent_canvas->window->height)
            );
        }
    }
}

static void cleanup(struct entity *entity, struct component_storage storage, void *data)
{
    struct ui_container *const container = storage.passive;

    list_destroy(&container->children);
    list_destroy(&container->on_left_click);
    list_destroy(&container->on_resize);
    list_destroy(&container->on_move);

    ui_text_destroy(&container->ui_text);

    string_destroy(container->ui_text.string);
}

void ui_container_draw(struct ui_container *container,
                       struct ui_render_cache *render_cache,
                       struct window *window)
{
    struct mat4x4 matrix = ui_render_calculate_matrix(
        &container->absolute_rect,
        container->depth,
        window
    );

    bool_t use_texture = FALSE;

    if (container->texture) {
        use_texture = TRUE;
        texture_bind(container->texture);
    }

    shader_uniform_int(render_cache->use_texture_uniform, use_texture);
    shader_uniform_int(render_cache->is_text_uniform, FALSE);
    shader_uniform_vec4f(render_cache->colour_uniform, container->colour);
    shader_uniform_mat4x4(render_cache->matrix_uniform, &matrix);

    mesh_draw(render_cache->quad);

    if (container->contains_text)
        ui_text_draw(&container->ui_text, container->depth, render_cache, window);
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

    callback_data->ecs          = resource_get(soul_instance, ECS_SERVICE);
    callback_data->font_service = resource_get(soul_instance, FONT_SERVICE);

    struct component_property_registry_info properties[] = {
        PROPERTY(ui_container, colour, "vec4f"),
        PROPERTY(ui_container, visible, "int"),
        PROPERTY(ui_container, rect, "rect"),
        PROPERTY(ui_container, layout, "int"),
        PROPERTY(ui_container, texture, "texture"),
        PROPERTY(ui_container, alignment, "int"),
        PROPERTY(ui_container, text, "string"),
        PROPERTY(ui_container, text_size, "int"),
        PROPERTY(ui_container, text_font, "font"),
        PROPERTY(ui_container, margins, "margins"),
        PROPERTY(ui_container, separation_margin, "int")
    };

    struct component_registry_info registry_info = {
        .name                   = UI_CONTAINER,
        .passive_storage_size   = sizeof(struct ui_container),
        .callbacks.init         = (component_callback_t)&init,
        .callbacks.entered_tree = (component_callback_t)&entered_tree,
        .callbacks.cleanup      = (component_callback_t)&cleanup,
        .callbacks.data         = callback_data,
        .properties             = properties,
        .property_count         = sizeof(properties)/sizeof(struct component_property_registry_info)
    };

    component_register(ecs_service, &registry_info);
}

static struct ui_container *get_most_shallow_modified(struct ui_container *container)
{
    if (container->rect.size.x == UI_MIN_SIZE || container->rect.size.y == UI_MIN_SIZE) {
        if (container->parent)
            return get_most_shallow_modified(container->parent);
        else
            return container;
    } else {
        return container;
    }
}

void calculate_container(struct ui_container *container)
{
    struct ui_container *most_shallow_modified = 0;

    if (container->parent)
        most_shallow_modified = get_most_shallow_modified(container->parent);
    else
        most_shallow_modified = get_most_shallow_modified(container);

    ui_container_calculate_children(most_shallow_modified);
}

void ui_container_set_rect(struct ui_container *container, struct ui_rect rect)
{
    container->rect = rect;

    if (rect.size.x > 0)
        container->absolute_rect.size.x = rect.size.x;

    if (rect.size.y > 0)
        container->absolute_rect.size.y = rect.size.y;

    calculate_container(container);

    /*
     * ui_container_calculate_children() will automatically dispatch on_resize for elements resized,
     * and this only occurs when a rect has a less than or equal to 0 size component.
     */
    if (rect.size.x > 0 && rect.size.y > 0)
        callbacks_dispatch(&container->on_resize, container);
}

void ui_container_set_text(struct ui_container *container, const char *text)
{
    if (container->text.chars)
        string_destroy(container->text);

    ui_text_set_string(&container->ui_text, text);

    container->contains_text    = TRUE;
    container->text             = string_create(text);

    calculate_container(container);
}

void ui_container_set_text_font(struct font_service *font_service,
                                struct ui_container *container,
                                struct font *font,
                                int height)
{
    ui_text_set_font(font_service, &container->ui_text, font, height);
    calculate_container(container);
}

static void calculate_min_size(struct ui_container *container)
{
    struct vec2i min = VEC2I_ZERO;

    list_for_each (struct ui_container *, p_child, container->children) {
        if ((*p_child)->rect.size.x == UI_MIN_SIZE || (*p_child)->rect.size.y == UI_MIN_SIZE)
            calculate_min_size(*p_child);

        if (UI_AXIS_P((*p_child)->rect.size, container->draw_axis) == UI_MAX_SIZE) {
            UI_AXIS_P((*p_child)->absolute_rect.size, container->draw_axis) = UI_AXIS_P(
                container->absolute_rect.size,
                container->draw_axis
            );
        }

        const struct vec2i child_size = (*p_child)->absolute_rect.size;

        if (container->draw_axis == VEC2I_XOFFSET) {
            min.x += child_size.x;
            min.y = max(min.y, child_size.y);
        } else {
            min.y += child_size.y;
            min.x = max(min.x, child_size.x);
        }
    }

    if (container->rect.size.x == UI_MIN_SIZE || container->rect.size.y == UI_MIN_SIZE) {
        if (container->rect.size.x == UI_MIN_SIZE) {
            if (container->contains_text) {
                int min_width = ui_text_calculate_min_width(container);
                min.x = max(min.x, min_width);
            }

            container->absolute_rect.size.x = min.x;
        }

        if (container->rect.size.y == UI_MIN_SIZE) {
            if (container->contains_text) {
                int min_height = ui_text_calculate_height(container);
                min.y = max(min.y, min_height);
            }

            container->absolute_rect.size.y = min.y;
        }

        callbacks_dispatch(&container->on_resize, container);
    }
}

static void calculate_min_sizes(struct ui_container *container)
{
    list_for_each (struct ui_container *, p_child, container->children) {
        if ((*p_child)->rect.size.x == UI_MIN_SIZE || (*p_child)->rect.size.y == UI_MIN_SIZE)
            calculate_min_size(*p_child);
    }
}

static int max_size_process_children(struct ui_container *container, int *available_pixels)
{
    int max_sizes = 0;

    int child_count = 0;

    list_for_each (struct ui_container *, p_child, container->children) {
        struct ui_container *const child = (struct ui_container *)(*p_child);

        if (UI_AXIS(child->rect.size, container->draw_axis) == UI_MAX_SIZE) {
            ++max_sizes;
        } else {
            int size = UI_AXIS(child->absolute_rect.size, container->draw_axis);
            *available_pixels -= size;
        }

        ++child_count;
    }

    *available_pixels -= max(child_count - 1, 0)*container->separation_margin;

    return max_sizes;
}

static struct vec2i calculate_ui_max_size(struct ui_container *container)
{
    if (container->draw_axis == -1)
        return VEC2I_ZERO;

    struct ui_rect draw_rect = container->absolute_rect;
    ui_margins_subtract(&draw_rect, &container->margins);

    int total_available_pixels = UI_AXIS(draw_rect.size, container->draw_axis);
    int available_pixels = total_available_pixels;

    int max_sizes = max_size_process_children(container, &available_pixels);

    struct vec2i max_size = draw_rect.size;

    const int children_size = total_available_pixels - available_pixels;
    container->children_size = children_size;

    if (max_sizes && available_pixels)
        UI_AXIS(max_size, container->draw_axis) = available_pixels/max_sizes;

    return max_size;
}

static void calculate_sizes(struct ui_container *container)
{
    calculate_min_sizes(container);

    struct vec2i max_size = calculate_ui_max_size(container);

    list_for_each (struct ui_container *, p_child, container->children) {
        if (container->layout != UI_LAYOUT_FREE) {
            bool_t resized = FALSE;

            if ((*p_child)->rect.size.x == UI_MAX_SIZE) {
                (*p_child)->absolute_rect.size.x = max_size.x;
                resized = TRUE;
            }

            if ((*p_child)->rect.size.y == UI_MAX_SIZE) {
                (*p_child)->absolute_rect.size.y = max_size.y;
                resized = TRUE;
            }

            if (resized)
                callbacks_dispatch(&(*p_child)->on_resize, (*p_child));
        }

        calculate_sizes(*p_child);
    }
}

static void calculate_positions_linear(struct ui_container *container)
{
    struct ui_rect rect = container->absolute_rect;
    ui_margins_subtract(&rect, &container->margins);

    struct vec2i cursor = ui_rect_calculate_cursor(
        &rect,
        container->alignment,
        container->children_size,
        container->draw_axis
    );

    list_for_each (struct ui_container *, p_child, container->children) {
        int cursor_offset = 0;

        (*p_child)->absolute_rect.position = cursor;

        ui_rect_align(
            &(*p_child)->absolute_rect,
            container->alignment,
            container->draw_axis
        );

        cursor_offset = UI_AXIS((*p_child)->absolute_rect.size, container->draw_axis);

        UI_AXIS(cursor, container->draw_axis) += cursor_offset + container->separation_margin;
    }
}

static void calculate_positions(struct ui_container *container, int depth)
{
    if (container->draw_axis != -1 && container->children.head)
        calculate_positions_linear(container);

    list_for_each (struct ui_container *, p_child, container->children) {
        (*p_child)->depth = depth + 1;
        calculate_positions(*p_child, depth + 1);
    }
}

static void calculate_text(struct ui_container *container)
{
    if (container->contains_text)
        ui_text_calculate(container);

    list_for_each (struct ui_container *, p_child, container->children) {
        calculate_text(*p_child);
    }
}

void ui_container_calculate_children(struct ui_container *container)
{
    calculate_sizes(container);
    calculate_positions(container, container->depth);
    calculate_text(container);
}

void ui_container_set_layout(struct ui_container *container, ui_layout_t layout)
{
    container->layout = layout;
    container->draw_axis = ui_axis_get_layout_axis(container->layout);
    calculate_container(container);
}

void ui_container_set_alignment(struct ui_container *container, ui_alignment_t alignment)
{
    container->alignment = alignment;
    calculate_positions(container, container->depth);
}

bool_t check_bounds(struct ui_container *container, struct vec2i point)
{
    int left_bound      = container->absolute_rect.position.x;
    int top_bound       = container->absolute_rect.position.y;
    int right_bound     = container->absolute_rect.size.x + left_bound;
    int bottom_bound    = container->absolute_rect.size.y + top_bound;

    if (point.x < left_bound)
        return FALSE;
    if (point.x > right_bound)
        return FALSE;
    if (point.y < top_bound)
        return FALSE;
    if (point.y > bottom_bound)
        return FALSE;

    return TRUE;
}

bool_t ui_container_test_mouse(struct ui_container *container, struct mouse_state *mouse)
{
    if (container->ignore_mouse_test || !check_bounds(container, mouse->position))
        return FALSE;

    list_for_each (struct ui_container *, p_child, container->children) {
        if (ui_container_test_mouse(*p_child, mouse))
            return TRUE;
    }

    if (mouse->buttons[MOUSE_LEFT])
        callbacks_dispatch(&container->on_left_click, container);

    return TRUE;
}