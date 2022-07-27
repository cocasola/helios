#ifndef UI_TEXT_H
#define UI_TEXT_H

#include "../string.h"
#include "../list.h"
#include "font.h"
#include "ui_render.h"

struct ui_character
{
    bool_t          whitespace;
    bool_t          is_tab;
    bool_t          new_line;
    struct glyph *  glyph;
    struct vec2i    position;
    struct ui_rect  absolute_rect;
};

struct ui_text
{
    struct string           string;
    struct font *           font;
    struct glyph_set *      glyph_set;
    struct ui_character *   characters;
    int                     character_count;
};

struct ui_container;

void    ui_text_init(struct ui_text *text);
void    ui_text_destroy(struct ui_text *text);
void    ui_text_draw(struct ui_text *text,
                     int depth,
                     struct ui_render_cache *render_cache,
                     struct window *window);
void    ui_text_set_string(struct ui_text *text, const char *string);
void    ui_text_set_font(struct font_service *service,
                         struct ui_text *text,
                         struct font *font,
                         int height);
int     ui_text_calculate_height(struct ui_container *container);
int     ui_text_calculate_min_width(struct ui_container *container);
void    ui_text_calculate(struct ui_container *container);

#endif // UI_TEXT_H