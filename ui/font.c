#include <stdio.h>

#include <soul/debug.h>
#include <soul/ui/font.h>

static void unload_freetype(struct font_service *service)
{
    FT_Done_FreeType(service->ft);
}

static void cleanup_font(struct font_service *service, struct font *font)
{
    list_for_each (struct glyph_set, set, font->glyph_sets) {
        for (int i = 0; i < FONT_GLYPH_COUNT; ++i) {
            if (set->glyphs[i].texture)
                texture_destroy(service->texture_service, set->glyphs[i].texture);
        }
    }

    list_destroy(&font->glyph_sets);

    string_destroy(font->name);
    FT_Done_Face(font->face);
}

static void deallocate_service(struct font_service *service)
{
    list_for_each (struct font, font, service->fonts) {
        cleanup_font(service, font);
    }

    list_destroy(&service->fonts);

    unload_freetype(service);
}

static void load_freetype(struct font_service *service)
{
    FT_Error result = FT_Init_FreeType(&service->ft);

    if (result) {
        debug_log(
            SEVERITY_ERROR,
            "Failed to create font_service. Could not initialize FreeType.\n"
        );

        abort();
    }
}

void font_service_create_resource(struct soul_instance *soul_instance)
{
    struct font_service *service = resource_create(
        soul_instance,
        FONT_SERVICE,
        sizeof(struct font_service),
        (resource_deallocator_t)&deallocate_service
    );

    load_freetype(service);

    list_init(&service->fonts, sizeof(struct font));
    service->texture_service = resource_get(soul_instance, TEXTURE_SERVICE);
}

struct font *font_load(struct font_service *service, const char *name)
{
    FT_Face face;

    char path[128];
    snprintf(path, 128, "/Windows/Fonts/%s", name);

    FT_Error result = FT_New_Face(service->ft, path, 0, &face);

    if (result) {
        debug_log(
            SEVERITY_WARNING,
            "Failed to load font '%s'.\n",
            name
        );

        return 0;
    }

    struct font *font = list_alloc(&service->fonts);

    font->name = string_create(name);
    font->face = face;
    list_init(&font->glyph_sets, sizeof(struct glyph_set));

    return font;
}

void font_unload(struct font_service *service, struct font *font)
{
    cleanup_font(service, font);
    list_remove(&service->fonts, font);
}

static struct texture *load_glyph_texture(struct font_service *service,
                                          FT_Bitmap *bitmap)
{
    struct texture_create_info texture_create_info = NEW_TEXTURE_CREATE_INFO;
    texture_create_info.channel_count       = 1;
    texture_create_info.pixels              = bitmap->buffer;
    texture_create_info.width               = bitmap->width;
    texture_create_info.height              = bitmap->rows;
    texture_create_info.generate_mip_maps   = FALSE;
    texture_create_info.no_memory_manage    = TRUE;
    texture_create_info.filter_mode         = TEXTURE_FILTERMODE_NEAREST;

    return texture_create(service->texture_service, &texture_create_info);
}

static void load_glyph(struct font_service *service,
                       struct font *font,
                       struct glyph_set *set,
                       int c)
{
    FT_Error r = FT_Load_Char(font->face, c, FT_LOAD_RENDER);

    if (r) {
        debug_log(
            SEVERITY_WARNING,
            "Failed to load glyph '%d' from '%s'.\n",
            c,
            font->name.chars
        );

        return;
    }

    FT_GlyphSlot glyph = font->face->glyph;

    if (!glyph->bitmap.buffer)
        return;

    set->glyphs[c].texture      = load_glyph_texture(service, &glyph->bitmap);
    set->glyphs[c].size         = vec2i(glyph->bitmap.width, glyph->bitmap.rows);
    set->glyphs[c].bearing      = vec2i(glyph->bitmap_left, glyph->bitmap_top);
    set->glyphs[c].advance      = glyph->advance.x/64.0;
    set->glyphs[c].draw_width   = set->glyphs[c].bearing.x + set->glyphs[c].size.x;
}

struct glyph_set *font_load_glyph_set(struct font_service *service,
                                      struct font *font,
                                      int height)
{
    struct glyph_set *set = list_alloc(&font->glyph_sets);

    FT_Set_Pixel_Sizes(font->face, 0, height);

    set->line_height    = font->face->size->metrics.height/64.0;
    set->height         = height;
    set->ascent         = font->face->size->metrics.ascender/64.0;
    set->descent        = font->face->size->metrics.descender/64.0;

    for (int c = 0; c < FONT_GLYPH_COUNT; ++c) {
        load_glyph(service, font, set, c);
    }

    return set;
}