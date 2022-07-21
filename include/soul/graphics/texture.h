#ifndef TEXTURE_H
#define TEXTURE_H

#include "../typedefs.h"
#include "../string.h"
#include "../callbacks.h"
#include "core.h"

typedef int texture_filtermode_t;
#define TEXTURE_FILTERMODE_NEAREST  0
#define TEXTURE_FILTERMODE_LINEAR   1

#define TEXTURE_SERVICE "texture_service"

struct texture_service
{
    struct list textures; // struct texture
    struct list render_targets; // struct render_target
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
    .flip               = TRUE                                  \
})

struct render_target
{
    struct string   name;
    struct texture *texture;
    struct list     on_resize; // struct callback, struct render_target *
    unsigned int    fbo;
    unsigned int    rbo;
};

struct render_target_create_info
{
    const char *            name;
    int                     width;
    int                     height;
    texture_filtermode_t    filter_mode;
    int                     channel_count;
};

#define NEW_RENDER_TARGET_CREATE_INFO ((struct render_target_create_info){  \
    .name           = "render_target",                                      \
    .width          = 0,                                                    \
    .height         = 0,                                                    \
    .filter_mode    = TEXTURE_FILTERMODE_LINEAR,                            \
    .channel_count  = 4                                                     \
})

void                    texture_service_create_resource(struct soul_instance *soul_instance);
struct texture *        texture_create(struct texture_service *texture_service,
                                       struct texture_create_info *create_info);
void                    texture_destroy(struct texture_service *texture_service,
                                        struct texture *texture);
void                    texture_resize(struct texture *texture, int width, int height);
void                    texture_bind(struct texture *texture);
struct render_target *  render_target_create(struct texture_service *texture_service,
                                             struct render_target_create_info *info);
void                    render_target_destroy(struct texture_service *texture_service,
                                              struct render_target *render_target);
void                    render_target_resize(struct render_target *render_target,
                                             int width,
                                             int height);
void                    render_target_unbind(void);
void                    render_target_bind(struct render_target *render_target);

#endif // TEXTURE_H