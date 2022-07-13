#ifndef ECS_H
#define ECS_H

#include "math/vector.h"

#include "list.h"
#include "string.h"
#include "core.h"

#define ECS_SERVICE "ecs_service"

struct component_reference;

struct entity
{
    struct string       name;
    struct context *    context;
    struct transform *  transform;
    struct entity *     parent;
    struct list         components; // struct component_reference
    struct list         children; // struct entity *
};

struct transform
{
    struct vec3f position;
    struct vec3f rotation;
    struct vec3f scale;
};

struct context
{
    struct string name;
};

#define COMPONENT struct entity *   entity;     \
                  struct transform *transform;

struct component
{
    COMPONENT
};

typedef void(*component_callback_t)(void *instance, void *data);

struct component_descriptor
{
    struct string           name;
    struct list             instances; // struct component
    void *                  callback_data;
    component_callback_t    init;
    component_callback_t    entered_tree;
    component_callback_t    cleanup;
};

struct component_reference
{
    struct component_descriptor *   descriptor;
    struct component *              instance;
};

struct ecs_service
{
    struct list     entities; // struct entity
    struct list     transforms; // struct transform
    struct list     contexts; // struct context
    struct list     components; // struct component_descriptor
    struct context *default_context;
};

struct component_registry_info
{
    const char *            name;
    void *                  callback_data;
    component_callback_t    init;
    component_callback_t    entered_tree;
    component_callback_t    cleanup;
    size_t                  struct_size;
};

void                            ecs_service_create_resource(struct soul_instance *instance);
struct entity *                 entity_create(struct ecs_service *ecs,
                                              const char *name,
                                              struct context *context,
                                              struct entity *parent);
void                            entity_destroy(struct ecs_service *ecs, struct entity *entity);
void *                          component_instance(struct ecs_service *ecs,
                                                   struct entity *entity,
                                                   const char *component);
void                            component_destroy_instance(struct ecs_service *ecs,
                                                           struct entity *entity,
                                                           void *component);
struct context *                context_create(struct ecs_service *ecs, const char *name);
void                            context_destroy(struct ecs_service *ecs, struct context *context);
struct component_descriptor *   component_register(struct ecs_service *ecs,
                                                   struct component_registry_info *registry_info);

#endif // ECS_H