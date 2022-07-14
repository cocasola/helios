#include <GL/glew.h>

#include <soul/execution_order.h>
#include <soul/math/matrix.h>
#include <soul/graphics/sprite.h>
#include <soul/graphics/shader.h>
#include <soul/graphics/mesh.h>
#include <soul/graphics/camera.h>

struct render_cache
{
    struct list *                   camera_instances;
    struct component_descriptor *   descriptor;
    struct mesh *                   quad;
    struct shader *                 sprite_shader;
    uniform_t                       matrix_uniform;
};

static struct mat4x4 calculate_matrix(struct transform *camera_transform,
                                      struct transform *sprite_transform,
                                      struct camera *camera)
{
    struct mat4x4 r = MAT4X4_IDENTITY;

    struct vec3f position = sub3f(sprite_transform->position, camera_transform->position);
    mat4x4_set_pos(&r, position);
    mat4x4_set_scale(&r, sprite_transform->scale);

    struct mat4x4 scale_matrix = MAT4X4_IDENTITY;

    mat4x4_set_scale(
        &scale_matrix,
        vec3f(
            1.0/camera->render_target.width/camera->size,
            1.0/camera->render_target.height/camera->size,
            0
        )
    );

    r = mul4x4(&scale_matrix, &r);

    return r;
}

static void render(struct render_cache *cache)
{
    list_for_each (struct camera, camera, *cache->camera_instances) {
        camera_bind(camera);

        list_for_each (struct sprite, sprite, cache->descriptor->instances) {
            texture_bind(sprite->texture);
            shader_bind(cache->sprite_shader);

            struct mat4x4 matrix = calculate_matrix(
                camera->transform,
                sprite->transform,
                camera
            );

            shader_uniform_mat4x4(cache->matrix_uniform, &matrix);

            mesh_draw(cache->quad);
        }
    }
}

static struct render_cache *create_render_cache(struct ecs_service *ecs_service,
                                                struct soul_instance *soul_instance,
                                                struct component_descriptor *descriptor)
{
    struct shader_service *shader_service = resource_get(soul_instance, SHADER_SERVICE);
    struct mesh_service *mesh_service = resource_get(soul_instance, MESH_SERVICE);

    struct render_cache *render_cache = resource_create(
        soul_instance,
        "sprite_render_cache",
        sizeof(struct render_cache),
        0
    );

    render_cache->descriptor        = descriptor;
    render_cache->quad              = mesh_service->primitives.quad;
    render_cache->sprite_shader     = shader_service->defaults.sprite;
    render_cache->matrix_uniform    = shader_get_uniform(render_cache->sprite_shader, "matrix");
    render_cache->camera_instances  = &component_match_descriptor(ecs_service, CAMERA)->instances;

    return render_cache;
}

void sprite_register_component(struct soul_instance *soul_instance)
{
    struct ecs_service *ecs_service = resource_get(soul_instance, ECS_SERVICE);

    struct component_registry_info registry_info = {
        .name           = SPRITE,
        .callback_data  = 0,
        .struct_size    = sizeof(struct sprite),
        .init           = 0,
        .entered_tree   = 0,
        .cleanup        = 0
    };

    struct component_descriptor *descriptor = component_register(ecs_service, &registry_info);

    struct render_cache *render_cache = create_render_cache(ecs_service, soul_instance, descriptor);

    ordered_callbacks_insert(
        &soul_instance->callbacks,
        (ordered_callback_t)render,
        EXECUTION_ORDER_RENDER,
        render_cache,
        FALSE
    );
}

void sprite_set_texture(struct sprite *sprite, struct texture *texture, bool_t match_size)
{
    sprite->texture = texture;

    if (match_size) {
        sprite->transform->scale.x = texture->width;
        sprite->transform->scale.y = texture->height;
    }
}