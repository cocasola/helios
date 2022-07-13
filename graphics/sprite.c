#include <GL/glew.h>

#include <soul/execution_order.h>
#include <soul/graphics/sprite.h>
#include <soul/graphics/shader.h>
#include <soul/graphics/mesh.h>

struct render_cache
{
    struct component_descriptor *   descriptor;
    struct mesh *                   quad;
    struct shader *                 sprite_shader;
};

static void deallocate_render_cache(struct render_cache *render_cache) { }

static void init(struct sprite *sprite, void *data) { }

static void entered_tree(struct sprite *sprite, void *data) { }

static void cleanup(struct sprite *sprite, void *data) { }

static void render(struct render_cache *cache)
{
    list_for_each (struct sprite, sprite, cache->descriptor->instances) {
        glActiveTexture(GL_TEXTURE0);
        texture_bind(sprite->texture);
        shader_bind(cache->sprite_shader);
        uniform_t texture_uniform = shader_get_uniform(cache->sprite_shader, "tex");
        shader_uniform_int(texture_uniform, 0);
        mesh_draw(cache->quad);
    }
}

void sprite_register_component(struct soul_instance *soul_instance)
{
    struct ecs_service *ecs_service = resource_get(soul_instance, ECS_SERVICE);

    struct component_registry_info registry_info = {
        .name           = SPRITE,
        .callback_data  = 0,
        .struct_size    = sizeof(struct sprite),
        .init           = (component_callback_t)&init,
        .entered_tree   = (component_callback_t)&entered_tree,
        .cleanup        = (component_callback_t)&cleanup
    };

    struct component_descriptor *descriptor = component_register(ecs_service, &registry_info);

    struct shader_service *shader_service = resource_get(soul_instance, SHADER_SERVICE);
    struct mesh_service *mesh_service = resource_get(soul_instance, MESH_SERVICE);

    struct render_cache *render_cache = resource_create(
        soul_instance,
        "sprite-render-cache",
        sizeof(struct render_cache),
        (resource_deallocator_t)&deallocate_render_cache
    );

    render_cache->descriptor    = descriptor;
    render_cache->quad          = mesh_service->primitives.quad;
    render_cache->sprite_shader = shader_service->defaults.sprite;

    ordered_callbacks_insert(
        &soul_instance->callbacks,
        (ordered_callback_t)render,
        EXECUTION_ORDER_RENDER,
        render_cache,
        FALSE
    );
}