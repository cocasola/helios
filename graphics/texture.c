#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>
#include <GL/glew.h>

#include <soul/debug.h>
#include <soul/graphics/texture.h>

static unsigned char *load_image(const char *path,
                                 int *width,
                                 int *height,
                                 int *channel_count,
                                 bool_t flip)
{
    if (flip)
        stbi_set_flip_vertically_on_load(1);
    else
        stbi_set_flip_vertically_on_load(0);

    unsigned char *pixels = stbi_load(path, width, height, channel_count, 0);

    return pixels;
}

static void cleanup_texture(struct texture *texture)
{
    glDeleteTextures(1, &texture->gl_texture);

    if (texture->read_write_enabled)
        free(texture->pixels);
}

static void service_deallocate(struct texture_service *service)
{
    list_for_each (struct texture, texture, service->textures) {
        cleanup_texture(texture);
    }

    list_destroy(&service->textures);
}

void texture_service_create_resource(struct soul_instance *soul_instance)
{
    struct texture_service *service = resource_create(
        soul_instance,
        TEXTURE_SERVICE,
        sizeof(struct texture_service),
        (resource_deallocator_t)&service_deallocate
    );

    list_init(&service->textures, sizeof(struct texture));
}

struct texture *texture_create(struct texture_service *texture_service,
                               struct texture_create_info *create_info)
{
    struct texture *texture = list_alloc(&texture_service->textures);

    texture->name               = string_create(create_info->name);
    texture->read_write_enabled = create_info->read_write_enabled;
    texture->no_memory_manage   = create_info->no_memory_manage;
    texture->width              = create_info->width;
    texture->height             = create_info->height;
    texture->channel_count      = create_info->channel_count;
    texture->filter_mode        = create_info->filter_mode;

    if (create_info->resource_path) {
        texture->pixels = load_image(
            create_info->resource_path,
            &texture->width,
            &texture->height,
            &texture->channel_count,
            create_info->flip
        );

        if (!texture->pixels) {
            debug_log(
                SEVERITY_WARNING,
                "Failed to create texture \"%s\". Could not load \"%s\".",
                texture->name.chars,
                create_info->resource_path
            );

            return 0;
        }
    }

    memset(texture->pixels, 255, 5000);

    GLenum filter_mode_enum;

    switch (texture->filter_mode) {
        case TEXTURE_FILTERMODE_NEAREST:
            filter_mode_enum = GL_NEAREST;
            break;

        case TEXTURE_FILTERMODE_LINEAR:
            filter_mode_enum = GL_LINEAR;
            break;
    }

    glGenTextures(1, &texture->gl_texture);
    glBindTexture(GL_TEXTURE_2D, texture->gl_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_mode_enum);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_mode_enum);

    GLenum channel_enum;

    switch (texture->channel_count) {
        case 1:
            channel_enum = GL_RED;
            break;

        case 2:
            channel_enum = GL_RGB;
            break;

        case 4:
            channel_enum = GL_RGBA;
            break;
    }

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        channel_enum,
        texture->width,
        texture->height,
        0,
        channel_enum,
        GL_UNSIGNED_BYTE,
        texture->pixels
    );

    if (create_info->generate_mip_maps)
        glGenerateMipmap(GL_TEXTURE_2D);

    if (!create_info->read_write_enabled && create_info->resource_path) {
        stbi_image_free(texture->pixels);
        texture->pixels = 0;
    }

    return texture;
}

void texture_destroy(struct texture_service *texture_service,
                     struct texture *texture)
{
    cleanup_texture(texture);
    list_remove(&texture_service->textures, texture);
}

void texture_bind(struct texture *texture)
{
    glBindTexture(GL_TEXTURE_2D, texture->gl_texture);
}