#include <limits.h>

#include <soul/ui/ui_text.h>
#include <soul/ui/ui_container.h>
#include <soul/math/macros.h>

#define DEFAULT_LINE_GAP            5
#define DEFAULT_SPACEWIDTH_RATIO    0.5

struct text_row
{
    int width;
    int count;
};

void ui_text_init(struct ui_text *text) { }

void ui_text_destroy(struct ui_text *text)
{
    if (text->characters)
        free(text->characters);
}

void ui_text_draw(struct ui_text *text,
                  int depth,
                  struct ui_render_cache *render_cache,
                  struct window *window)
{
    for (int i = 0; i < text->character_count; ++i) {
        if (!text->characters[i].glyph)
            continue;

        struct mat4x4 matrix = ui_render_calculate_matrix(
            &text->characters[i].absolute_rect,
            depth + 1,
            window
        );

        texture_bind(text->characters[i].glyph->texture);

        shader_uniform_int(render_cache->is_text_uniform, TRUE);
        shader_uniform_int(render_cache->use_texture_uniform, TRUE);
        shader_uniform_mat4x4(render_cache->matrix_uniform, &matrix);

        mesh_draw(render_cache->quad);
    }
}

static void create_glyphs(struct ui_text *text)
{
    text->characters = calloc(1, text->string.length*sizeof(struct ui_character));
    text->character_count = text->string.length;

    char *const string = text->string.chars;

    for (int i = 0; i < text->string.length; ++i) {
        switch (string[i]) {
            case ' ':
                text->characters[i].whitespace = TRUE;
                break;

            case '\t':
                text->characters[i].whitespace = TRUE;
                text->characters[i].is_tab = TRUE;
                break;

            case '\n':
                text->characters[i].new_line = TRUE;
                break;

            default:
                text->characters[i].glyph = text->glyph_set->glyphs + string[i];
                text->characters[i].absolute_rect.size = text->characters[i].glyph->size;
                break;
        }
    }
}

void ui_text_set_string(struct ui_text *text, const char *string)
{
    if (text->characters) {
        string_destroy(text->string);
        free(text->characters);
    }

    text->string = string_create(string);

    create_glyphs(text);
}

void ui_text_set_font(struct font_service *service,
                      struct ui_text *text,
                      struct font *font,
                      int height)
{
    struct glyph_set *set = 0;

    list_for_each (struct glyph_set, iter, font->glyph_sets) {
        if (iter->height == height) {
            set = iter;
            break;
        }
    }

    if (!set)
        set = font_load_glyph_set(service, font, height);

    bool_t recreate_glyphs = text->characters && text->string.chars; 

    text->font      = font;
    text->glyph_set = set;

    if (recreate_glyphs) {
        free(text->characters);
        create_glyphs(text);
    }
}

static struct list calculate_rows(struct ui_text *text, int max_width)
{
    struct list rows;
    list_init(&rows, sizeof(struct text_row));

    int last_whitespace = 0;
    int cursor = 0;
    int trailing = 0;

    bool_t last_was_whitespace = FALSE;

    for (int i = 0; i < text->character_count; ++i) {
        struct glyph *const glyph = text->characters[i].glyph;

        if (!glyph) {
            if (text->characters[i].whitespace) {
                last_whitespace = i;
                cursor += DEFAULT_SPACEWIDTH_RATIO*text->glyph_set->height;
                last_was_whitespace = TRUE;
            } else if (text->characters[i].new_line) {
                struct text_row row = {
                    cursor - trailing,
                    i
                };

                list_push(&rows, &row);

                last_whitespace = 0;
                cursor = 0;
                trailing = 0;
                last_was_whitespace = FALSE;
            }

            continue;
        }

        int width = glyph->size.x;

        if (!last_was_whitespace && cursor)
            width += glyph->bearing.x;

        if (cursor + width > max_width) {
            // If the cursor is already 0, this text cannot fit period.
            if (!cursor)
                return rows;

            if (!last_whitespace) {
                struct text_row row = {
                    cursor - trailing,
                    i
                };

                list_push(&rows, &row);
                --i;
            } else {
                struct text_row row = {
                    cursor - trailing,
                    last_whitespace
                };

                list_push(&rows, &row);

                i = last_whitespace;
                last_whitespace = 0;
            }

            cursor = 0;
            trailing = 0;
        } else {
            cursor += glyph->advance;
            trailing = glyph->advance - width;
        }

        last_was_whitespace = FALSE;
    }

    struct text_row row = {
        cursor - trailing,
        text->character_count
    };

    list_push(&rows, &row);

    return rows;
}

static int calculate_text_height(struct ui_container *container, struct list *rows)
{
    int row_count = 0;

    list_for_each (int, p_new_line, *rows) {
        ++row_count;
    }

    int gaps = max(row_count - 1, 0);

    return row_count*(container->ui_text.glyph_set->line_height) + gaps*DEFAULT_LINE_GAP;
}

int ui_text_calculate_height(struct ui_container *container)
{
    struct list rows = calculate_rows(&container->ui_text, container->absolute_rect.size.x);

    int height = calculate_text_height(container, &rows);

    list_destroy(&rows);

    return height;
}

int ui_text_calculate_min_width(struct ui_container *container)
{
    struct list rows = calculate_rows(&container->ui_text, INT_MAX);

    int width;
    struct text_row *row = list_get_head(&rows);

    if (row)
        width = row->width;

    list_destroy(&rows);

    return width;
}

static struct vec2i init_cursor(struct ui_rect *rect, ui_alignment_t alignment, int draw_size)
{
    struct vec2i cursor = rect->position;

    switch (alignment) {
        case UI_ALIGNMENT_LEFT:
            cursor.y += rect->size.y/2;
            cursor.y -= draw_size/2;
            break;

        case UI_ALIGNMENT_CENTER:
            cursor.y += rect->size.y/2;
            cursor.y -= draw_size/2;
            break;
    }

    return cursor;
}

void ui_text_calculate(struct ui_container *container)
{
    struct list rows = calculate_rows(&container->ui_text, container->absolute_rect.size.x);
    struct text_row *row = list_get_head(&rows);

    struct ui_rect rect = container->absolute_rect;
    ui_margins_subtract(&rect, &container->margins);

    int draw_size = calculate_text_height(container, &rows);

    struct vec2i cursor = init_cursor(&rect, container->alignment, draw_size);
    cursor.y += container->ui_text.glyph_set->ascent;

    bool_t last_was_whitespace = TRUE;

    for (int i = 0; i < container->ui_text.character_count; ++i) {
        struct ui_character *const c = container->ui_text.characters + i;

        if (row && row->width == i) {
            cursor.x = container->absolute_rect.position.x;
            cursor.y += container->ui_text.glyph_set->line_height + DEFAULT_LINE_GAP;

            row = list_get_next(row);

            if (c->whitespace)
                continue;
        }

        if (c->glyph) {
            c->absolute_rect.position = cursor;

            if (!last_was_whitespace)
                c->absolute_rect.position.x += c->glyph->bearing.x;

            c->absolute_rect.position.y -= c->glyph->bearing.y;

            cursor.x += c->glyph->advance;

            last_was_whitespace = FALSE;
        } else if (c->whitespace) {
            cursor.x += container->ui_text.glyph_set->height*DEFAULT_SPACEWIDTH_RATIO;
            last_was_whitespace = TRUE;
        }
    }

    list_destroy(&rows);
}