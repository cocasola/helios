#ifndef ECS_H
#define ECS_H

#include "math/transform.h"

#include "list.h"
#include "string.h"
#include "core.h"
#include "property_serialization.h"

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
    struct list         on_child_added; // struct callback, struct entity *
};

struct context
{
    struct string name;
};

struct component_storage
{
    void *active;
    void *passive;
};

typedef void(*component_callback_t)(
    struct entity *entity,
    struct component_storage storage,
    void *data
);

struct component_property
{
    size_t                      offset;
    struct property_serializer  serializer;
};

struct component_descriptor
{
    struct string           name;
    struct list             active_storage; // void
    struct list             passive_storage; // void
    component_callback_t    init;
    component_callback_t    entered_tree;
    component_callback_t    cleanup;
    void *                  callback_data;
    struct string_map       properties; // struct component_property
};

struct component_reference
{
    struct component_descriptor *   descriptor;
    struct component_storage        storage;
};

struct ecs_service
{
    struct list         entities; // struct entity
    struct list         transforms; // struct transform
    struct list         contexts; // struct context
    struct list         components; // struct component_descriptor
    struct context *    default_context;
    struct string_map   property_serializers; // struct serializer
};

struct component_callbacks
{
    component_callback_t    init;
    component_callback_t    entered_tree;
    component_callback_t    cleanup;
    void *                  data;
};

struct component_property_registry_info
{
    const char *    name;
    const char *    type;
    size_t          offset;
};

#define PROPERTY(storage, name, type) ((struct component_property_registry_info) {  \
    #name,                                                                          \
    type,                                                                           \
    offsetof(struct storage, name)                                                  \
})

struct component_registry_info
{
    const char *                                name;
    size_t                                      active_storage_size;
    size_t                                      passive_storage_size;
    struct component_callbacks                  callbacks;
    struct component_property_registry_info *   properties;
    int                                         property_count;
};

void                            ecs_service_create_resource(struct soul_instance *instance);
struct entity *                 entity_create(struct ecs_service *ecs,
                                              const char *name,
                                              struct context *context,
                                              struct entity *parent);
void                            entity_destroy(struct ecs_service *ecs, struct entity *entity);
struct entity *                 entity_load(struct ecs_service *ecs,
                                            const char *path,
                                            struct context *context,
                                            struct entity *parent);
struct component_storage        component_instance(struct ecs_service *ecs,
                                                   struct entity *entity,
                                                   const char *component);
void                            component_destroy_instance(struct ecs_service *ecs,
                                                           struct entity *entity,
                                                           struct component_storage storage);
struct component_storage        component_get_storage(struct ecs_service *ecs,
                                                      struct entity *entity,
                                                      const char *name);
struct context *                context_create(struct ecs_service *ecs, const char *name);
void                            context_destroy(struct ecs_service *ecs, struct context *context);
struct component_descriptor *   component_register(struct ecs_service *ecs,
                                                   struct component_registry_info *registry_info);
struct component_descriptor *   component_match_descriptor(struct ecs_service *ecs,
                                                           const char *component);

#endif // ECS_H