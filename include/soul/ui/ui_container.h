#ifndef UI_CONTAINER_H
#define UI_CONTAINER_H

struct ui_container;

#include "../callbacks.h"
#include "../core.h"
#include "ui_render.h"
#include "ui_text.h"
#include "window.h"
#include "ui_alignment.h"
#include "ui_layout.h"
#include "ui_rect.h"
#include "ui_margins.h"
#include "ui_axis.h"
#include "user_input.h"

#define UI_CONTAINER "ui_container"

#define UI_MAX_SIZE -1
#define UI_MIN_SIZE 0

struct ui_container
{
    struct entity *         entity;

    struct ui_container *   parent;
    struct list             children; // struct ui_container *
    int                     children_size;

    struct ui_rect          rect;
    struct ui_rect          absolute_rect;
    int                     depth;

    ui_layout_t             layout;
    ui_alignment_t          alignment;
    struct ui_margins       margins;
    int                     separation_margin;
    ui_axis_t               draw_axis;

    struct vec4f            colour;
    struct vec4f            current_colour;
    struct texture *        texture;
    bool_t                  visible;

    struct string           text;
    int                     text_size;
    struct ui_text          ui_text;
    bool_t                  contains_text;
    struct font *           text_font;

    struct vec4f            hover_colour;
    float                   hover_timer;
    float                   hover_transition_time;

    bool_t                  ignore_mouse_test;

    struct list             on_left_click; // struct callback, struct ui_container *
    struct list             on_resize; // struct callback, struct ui_container *
    struct list             on_move; // struct callback, struct ui_container *
};

void    ui_container_register_component(struct soul_instance *soul_instance);
void    ui_container_set_rect(struct ui_container *container, struct ui_rect rect);
void    ui_container_set_text(struct ui_container *container, const char *text);
void    ui_container_set_text_font(struct font_service *font_service,
                                   struct ui_container *container,
                                   struct font *font,
                                   int height);
void    ui_container_draw(struct ui_container *container,
                          struct ui_render_cache *render_cache,
                          struct window *window);
void    ui_container_calculate_children(struct ui_container *container);
void    ui_container_set_layout(struct ui_container *container, ui_layout_t layout);
void    ui_container_set_alignment(struct ui_container *container, ui_alignment_t alignment);
bool_t  ui_container_test_mouse(struct ui_container *container, struct mouse_state *mouse);

#endif // UI_CONTAINER_H