#include <soul/ecs.h>
#include <soul/debug.h>

static void destroy_component_instance(struct component_reference ref)
{
    if (ref.descriptor->cleanup)
        ref.descriptor->cleanup(ref.instance, ref.descriptor->callback_data);

    list_remove(&ref.descriptor->instances, ref.instance);
}

static void cleanup_entity(struct ecs_service *ecs, struct entity *entity)
{
    string_destroy(entity->name);
    list_remove(&ecs->transforms, entity->transform);

    list_for_each (struct component_reference, ref, entity->components) {
        destroy_component_instance(*ref);
    }

    list_destroy(&entity->children);
    list_destroy(&entity->components);
}

static void cleanup_context(struct ecs_service *ecs, struct context *context)
{
    string_destroy(context->name);
}

static void cleanup_component_descriptor(struct ecs_service *ecs,
                                         struct component_descriptor *descriptor)
{
    list_destroy(&descriptor->instances);
    string_destroy(descriptor->name);
}

static void deallocate_service(struct ecs_service *ecs)
{
    list_for_each (struct entity, entity, ecs->entities) {
        cleanup_entity(ecs, entity);
    }

    list_for_each (struct context, context, ecs->contexts) {
        cleanup_context(ecs, context);
    }

    list_for_each (struct component_descriptor, descriptor, ecs->components) {
        cleanup_component_descriptor(ecs, descriptor);
    }

    list_destroy(&ecs->components);
    list_destroy(&ecs->contexts);
    list_destroy(&ecs->entities);
    list_destroy(&ecs->transforms);
}

void ecs_service_create_resource(struct soul_instance *instance)
{
    struct ecs_service *ecs = resource_create(
        instance,
        ECS_SERVICE,
        sizeof(struct ecs_service),
        (resource_deallocator_t)&deallocate_service
    );

    list_init(&ecs->components, sizeof(struct component_descriptor));
    list_init(&ecs->contexts, sizeof(struct context));
    list_init(&ecs->entities, sizeof(struct entity));
    list_init(&ecs->transforms, sizeof(struct transform));

    ecs->default_context = context_create(ecs, "default");
}

struct entity *entity_create(struct ecs_service *ecs,
                             const char *name,
                             struct context *context,
                             struct entity *parent)
{
    struct entity *entity = list_alloc(&ecs->entities);

    struct transform *transform = list_alloc(&ecs->transforms);

    entity->context     = (context) ? context : ecs->default_context;
    entity->parent      = parent;
    entity->transform   = transform;
    entity->name        = string_create(name);

    entity->transform->scale = VEC3F_ONE;

    if (parent)
        list_push(&parent->children, &entity);

    list_init(&entity->components, sizeof(struct component_reference));
    list_init(&entity->children, sizeof(struct enitity *));

    return entity;
}

void entity_destroy(struct ecs_service *ecs, struct entity *entity)
{
    list_for_each (struct entity *, p_entity, entity->children) {
        entity_destroy(ecs, *p_entity);
    }

    if (entity->parent) {
        list_remove_value(&entity->parent->children, &entity);
    }

    cleanup_entity(ecs, entity);
    list_remove(&ecs->entities, entity);
}

static struct component_reference init_component_instance(struct component *instance,
                                                          struct entity *entity,
                                                          struct component_descriptor *descriptor)
{
    instance->entity    = entity;
    instance->transform = entity->transform;

    if (descriptor->init)
        descriptor->init(instance, descriptor->callback_data);

    if (descriptor->entered_tree)
        descriptor->entered_tree(instance, descriptor->callback_data);

    struct component_reference ref = {
        .descriptor = descriptor,
        .instance   = instance
    };

    return ref;
}

void *component_instance(struct ecs_service *ecs, struct entity *entity, const char *component)
{
    struct component_descriptor *descriptor = component_match_descriptor(ecs, component);

#ifdef DEBUG
    if (!descriptor) {
        debug_log(
            SEVERITY_ERROR,
            "Failed to instance '%s' on '%s'. Component descriptor could not be matched.",
            entity->name.chars,
            component
        );

        abort();
    }
#endif

    struct component *instance = list_alloc(&descriptor->instances);
    struct component_reference ref = init_component_instance(instance, entity, descriptor);

    list_push(&entity->components, &ref);

    return instance;
}

void component_destroy_instance(struct ecs_service *ecs, struct entity *entity, void *component)
{
    list_for_each (struct component_reference, ref, entity->components) {
        if (ref->instance == component) {
            destroy_component_instance(*ref);
            list_remove(&entity->components, ref);

            break;
        }
    }
}

struct context *context_create(struct ecs_service *ecs, const char *name)
{
    struct context *context = list_alloc(&ecs->contexts);
    context->name = string_create(name);

    return context;
}

void context_destroy(struct ecs_service *ecs, struct context *context)
{
    cleanup_context(ecs, context);
    list_remove(&ecs->contexts, context);
}

struct component_descriptor *component_register(struct ecs_service *ecs,
                                                struct component_registry_info *info)
{
    struct component_descriptor *descriptor = list_alloc(&ecs->components);

    list_init(&descriptor->instances, info->struct_size);

    descriptor->name            = string_create(info->name);
    descriptor->callback_data   = info->callback_data;
    descriptor->init            = info->init;
    descriptor->entered_tree    = info->entered_tree;
    descriptor->cleanup         = info->cleanup;

    return descriptor;
}

struct component_descriptor *component_match_descriptor(struct ecs_service *ecs,
                                                        const char *component)
{
    list_for_each (struct component_descriptor, iter, ecs->components) {
        if (string_eq_ptr(iter->name.chars, component))
            return iter;
    }

    return 0;
}