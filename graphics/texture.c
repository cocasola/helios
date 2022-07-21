#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>
#include <GL/glew.h>

#include <soul/debug.h>
#include <soul/debug.h>
#include <soul/string.h>
#include <soul/graphics/texture.h>

static unsigned char *load_image(const char *name,
                                 const char *path,
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

    if (!pixels) {
        debug_log(
            SEVERITY_WARNING,
            "Failed to create texture \"%s\". Could not load \"%s\".",
            name,
            path
        );

        return 0;
    }

    return pixels;
}

static void cleanup_render_target(struct texture_service *texture_service,
                                  struct render_target *render_target)
{
    string_destroy(render_target->name);
    list_destroy(&render_target->on_resize);
    texture_destroy(texture_service, render_target->texture);
    glDeleteFramebuffers(1, &render_target->fbo);
}

static void cleanup_texture(struct texture *texture)
{
    glDeleteTextures(1, &texture->gl_texture);

    if (texture->read_write_enabled)
        free(texture->pixels);

    string_destroy(texture->name);
}

static void service_deallocate(struct texture_service *service)
{
    list_for_each (struct render_target, render_target, service->render_targets) {
        cleanup_render_target(service, render_target);
    }

    list_for_each (struct texture, texture, service->textures) {
        cleanup_texture(texture);
    }

    list_destroy(&service->textures);
    list_destroy(&service->render_targets);
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
    list_init(&service->render_targets, sizeof(struct render_target));
}

GLenum get_gl_filtermode_enum(texture_filtermode_t filter_mode)
{
    switch (filter_mode) {
        case TEXTURE_FILTERMODE_NEAREST:
            return GL_NEAREST;
            break;

        case TEXTURE_FILTERMODE_LINEAR:
            return GL_LINEAR;
            break;
    }
}

GLenum get_gl_channel_enum(int channel_count)
{
    switch (channel_count) {
        case 1:
            return GL_RED;
            break;

        case 2:
            return GL_RGB;
            break;

        case 4:
            return GL_RGBA;
            break;
    }
}

static void create_gl_resource(struct texture *texture, struct texture_create_info *create_info)
{
    GLenum filter_mode_enum = get_gl_filtermode_enum(texture->filter_mode);
    GLenum channel_enum = get_gl_channel_enum(texture->channel_count);

    glGenTextures(1, &texture->gl_texture);
    glBindTexture(GL_TEXTURE_2D, texture->gl_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_mode_enum);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_mode_enum);

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
            create_info->name,
            create_info->resource_path,
            &texture->width,
            &texture->height,
            &texture->channel_count,
            create_info->flip
        );

        if (!texture->pixels)
            return 0;
    }

    create_gl_resource(texture, create_info);

    if (!create_info->read_write_enabled && create_info->resource_path) {
        stbi_image_free(texture->pixels);
        texture->pixels = 0;
    }

    return texture;
}

void texture_destroy(struct texture_service *texture_service, struct texture *texture)
{
    cleanup_texture(texture);
    list_remove(&texture_service->textures, texture);
}

void texture_resize(struct texture *texture, int width, int height)
{
    texture->width = width;
    texture->height = height;

    if (!texture->no_memory_manage)
        free(texture->pixels);

    size_t bytes = width*height*texture->channel_count*sizeof(unsigned char);
    texture->pixels = calloc(1, bytes);

    GLenum channel = get_gl_channel_enum(texture->channel_count);

    glBindTexture(GL_TEXTURE_2D, texture->gl_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, channel, width, height, 0, channel, GL_UNSIGNED_BYTE, 0);
}

void texture_bind(struct texture *texture)
{
    glBindTexture(GL_TEXTURE_2D, texture->gl_texture);
}

static void create_rbo(struct render_target *render_target,
                       struct render_target_create_info *info)
{
    glGenRenderbuffers(1, &render_target->rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, render_target->rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, info->width, info->height);  
}

static void create_fbo(struct render_target *render_target,
                       struct render_target_create_info *info)
{
    glGenFramebuffers(1, &render_target->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, render_target->fbo);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        render_target->texture->gl_texture,
        0
    );

    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER,
        render_target->rbo
    );

    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);

#ifdef DEBUG
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        debug_log(
            SEVERITY_ERROR,
            "Failed to create render_target '%s'. Invalid framebuffer.\n",
            info->name
        );
    }
#endif // DEBUG

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

struct render_target *render_target_create(struct texture_service *texture_service,
                                           struct render_target_create_info *info)
{
    struct render_target *render_target = list_alloc(&texture_service->render_targets);

    render_target->name = string_create(info->name);

    struct texture_create_info texture_create_info = NEW_TEXTURE_CREATE_INFO;
    texture_create_info.channel_count      = info->channel_count;
    texture_create_info.width              = info->width;
    texture_create_info.height             = info->height;
    texture_create_info.name               = info->name;
    texture_create_info.read_write_enabled = FALSE;

    render_target->texture = texture_create(texture_service, &texture_create_info);

    create_rbo(render_target, info);
    create_fbo(render_target, info);

    list_init(&render_target->on_resize, sizeof(struct callback));

    return render_target;
}

void render_target_destroy(struct texture_service *texture_service,
                           struct render_target *render_target)
{
    cleanup_render_target(texture_service, render_target);
    list_remove(&texture_service->render_targets, render_target);
}

void render_target_resize(struct render_target *render_target, int width, int height)
{
    texture_resize(render_target->texture, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, render_target->fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, render_target->rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    callbacks_dispatch(&render_target->on_resize, render_target);
}

void render_target_unbind(void)
{
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void render_target_bind(struct render_target *render_target)
{
    glBindFramebuffer(GL_FRAMEBUFFER, render_target->fbo);
    glViewport(0, 0, render_target->texture->width, render_target->texture->height);
}