#include <soul/ui/ui_text.h>
#include <soul/ui/ui_container.h>

#define DEFAULT_LINE_GAP            5
#define DEFAULT_SPACEWIDTH_RATIO    0.5

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
    text->string = string_create(string);

    if (text->characters)
        free(text->characters);

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

static struct list calculate_new_lines(struct ui_text *text, struct ui_container *container)
{
    struct list new_lines;
    list_init(&new_lines, sizeof(int));

    int last_whitespace = 0;
    int cursor = 0;

    bool_t last_was_whitespace = FALSE;

    for (int i = 0; i < text->character_count; ++i) {
        struct glyph *const glyph = text->characters[i].glyph;

        if (!glyph) {
            if (text->characters[i].whitespace) {
                last_whitespace = i;
                cursor += DEFAULT_SPACEWIDTH_RATIO*text->glyph_set->height;
                last_was_whitespace = TRUE;
            } else if (text->characters[i].new_line) {
                list_push(&new_lines, &i);

                last_whitespace = 0;
                cursor = 0;
                last_was_whitespace = FALSE;
            }

            continue;
        }

        int width = glyph->size.x;

        if (!last_was_whitespace && cursor)
            width += glyph->bearing.x;

        if (cursor + width >= container->absolute_rect.size.x) {
            if (!last_whitespace) {
                list_push(&new_lines, &i);
                --i;
            } else {
                list_push(&new_lines, &last_whitespace);

                i = last_whitespace;
                last_whitespace = 0;
            }

            cursor = 0;
        } else {
            cursor += glyph->advance;
        }

        last_was_whitespace = FALSE;
    }

    return new_lines;
}

int ui_text_calculate_height(struct ui_container *container)
{
    struct list new_lines = calculate_new_lines(&container->text, container);
    int new_line_count = 0;

    list_for_each (int, p_new_line, new_lines) {
        ++new_line_count;
    }

    list_destroy(&new_lines);

    const int line_height = container->text.glyph_set->line_height;

    return line_height + new_line_count*(line_height + DEFAULT_LINE_GAP);
}

void ui_text_calculate(struct ui_container *container)
{
    struct list new_lines = calculate_new_lines(&container->text, container);
    int *next_new_line = list_get_head(&new_lines);

    struct vec2i cursor = container->absolute_rect.position;
    cursor.y += container->text.glyph_set->ascent;

    bool_t last_was_whitespace = TRUE;

    for (int i = 0; i < container->text.character_count; ++i) {
        struct ui_character *const c = container->text.characters + i;

        if (next_new_line && *next_new_line == i) {
            cursor.x = container->absolute_rect.position.x;
            cursor.y += container->text.glyph_set->line_height + DEFAULT_LINE_GAP;

            next_new_line = list_get_next(next_new_line);

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
            cursor.x += container->text.glyph_set->height*DEFAULT_SPACEWIDTH_RATIO;
            last_was_whitespace = TRUE;
        }
    }

    list_destroy(&new_lines);
}