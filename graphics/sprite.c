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
    struct list *                   sprite_instances;
    struct mesh *                   quad;
    struct shader *                 shader;
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
            1.0/camera->render_target->texture->width/camera->size,
            1.0/camera->render_target->texture->height/camera->size,
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

        list_for_each (struct sprite, sprite, *cache->sprite_instances) {
            texture_bind(sprite->texture);
            shader_bind(cache->shader);

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

static void init(struct entity *entity, struct component_storage storage, void *data)
{
    struct sprite *const sprite = storage.passive;

    sprite->transform = entity->transform;
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

    struct component_descriptor *camera_descriptor = component_match_descriptor(
        ecs_service,
        CAMERA
    );

    render_cache->sprite_instances  = &descriptor->passive_storage;
    render_cache->quad              = mesh_service->primitives.quad;
    render_cache->shader            = shader_service->defaults.sprite;
    render_cache->matrix_uniform    = shader_get_uniform(render_cache->shader, "matrix");
    render_cache->camera_instances  = &camera_descriptor->passive_storage;

    return render_cache;
}

void sprite_register_component(struct soul_instance *soul_instance)
{
    struct ecs_service *ecs_service = resource_get(soul_instance, ECS_SERVICE);

    struct component_registry_info registry_info = {
        .name                   = SPRITE,
        .passive_storage_size   = sizeof(struct sprite),
        .callbacks.init         = (component_callback_t)&init
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