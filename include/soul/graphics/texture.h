#ifndef TEXTURE_H
#define TEXTURE_H

#include "../typedefs.h"
#include "../string.h"

#include "core.h"

typedef int texture_filtermode_t;
#define TEXTURE_FILTERMODE_NEAREST  0
#define TEXTURE_FILTERMODE_LINEAR   1

#define TEXTURE_SERVICE "texture_service"

struct texture_service
{
    struct list textures; // struct texture
};

struct texture
{
    struct string           name;
    int                     width;
    int                     height;
    int                     channel_count;
    bool_t                  read_write_enabled;
    unsigned char *         pixels;
    bool_t                  no_memory_manage;
    unsigned int            gl_texture;
    texture_filtermode_t    filter_mode;
};

struct texture_create_info
{
    const char *            name;
    const char *            resource_path;
    unsigned char *         pixels;
    int                     width;
    int                     height;
    int                     channel_count;
    texture_filtermode_t    filter_mode;
    bool_t                  generate_mip_maps;
    bool_t                  read_write_enabled;
    bool_t                  no_memory_manage;
    bool_t                  flip;
};

#define NEW_TEXTURE_CREATE_INFO ((struct texture_create_info){  \
    .name               = "texture",                            \
    .resource_path      = 0,                                    \
    .pixels             = 0,                                    \
    .width              = 0,                                    \
    .height             = 0,                                    \
    .channel_count      = 0,                                    \
    .generate_mip_maps  = FALSE,                                \
    .filter_mode        = TEXTURE_FILTERMODE_LINEAR,            \
    .read_write_enabled = FALSE,                                \
    .no_memory_manage   = FALSE,                                \
    .flip               = FALSE                                 \
})

void            texture_service_create_resource(struct soul_instance *soul_instance);
struct texture *texture_create(struct texture_service *texture_service,
                               struct texture_create_info *create_info);
void            texture_destroy(struct texture_service *texture_service,
                                struct texture *texture);
void            texture_bind(struct texture *texture);

#endif // TEXTURE_H