#ifndef FONT_H
#define FONT_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../typedefs.h"
#include "../string.h"
#include "../list.h"
#include "../core.h"
#include "../math/vector.h"
#include "../graphics/texture.h"

#define FONT_SERVICE        "font_service"
#define FONT_GLYPH_COUNT    128

struct glyph
{
    struct texture *texture;
    struct vec2i    size;
    struct vec2i    bearing;
    int             advance;
    int             draw_width;
};

struct glyph_set
{
    struct glyph    glyphs[FONT_GLYPH_COUNT];
    int             height;
    int             line_height;
    int             ascent;
    int             descent;
};

struct font
{
    struct string   name;
    struct list     glyph_sets; // struct glyph_set
    FT_Face         face;
};

struct font_service
{
    FT_Library              ft;
    struct texture_service *texture_service;
    struct list             fonts; // struct font
};

void                font_service_create_resource(struct soul_instance *soul_instance);
struct font *       font_load(struct font_service *service, const char *name);
void                font_unload(struct font_service *service, struct font *font);
struct glyph_set *  font_load_glyph_set(struct font_service *service,
                                        struct font *font,
                                        int height);

#endif // FONT_H