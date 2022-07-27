#include <soul/ecs.h>
#include <soul/json.h>
#include <soul/file.h>
#include <soul/debug.h>
#include <soul/callbacks.h>

static void destroy_component_instance(struct entity *entity, struct component_reference ref)
{
    if (ref.descriptor->cleanup)
        ref.descriptor->cleanup(entity, ref.storage, ref.descriptor->callback_data);

    if (ref.descriptor->active_storage.data_size)
        list_remove(&ref.descriptor->active_storage, ref.storage.active);

    if (ref.descriptor->passive_storage.data_size)
        list_remove(&ref.descriptor->passive_storage, ref.storage.passive);
}

static void cleanup_entity(struct ecs_service *ecs, struct entity *entity)
{
    string_destroy(entity->name);
    list_remove(&ecs->transforms, entity->transform);

    list_for_each (struct component_reference, ref, entity->components) {
        destroy_component_instance(entity, *ref);
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
    if (descriptor->active_storage.data_size)
        list_destroy(&descriptor->active_storage);

    if (descriptor->passive_storage.data_size)
        list_destroy(&descriptor->passive_storage);

    string_destroy(descriptor->name);

    string_map_destroy(&descriptor->properties);
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

    string_map_destroy(&ecs->property_serializers);
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

    string_map_init(&ecs->property_serializers, sizeof(struct property_serializer));
}

struct entity *entity_create(struct ecs_service *ecs,
                             const char *name,
                             struct context *context,
                             struct entity *parent)
{
    struct entity *entity = list_alloc(&ecs->entities);

    struct transform *transform = list_alloc(&ecs->transforms);

    entity->parent      = parent;
    entity->transform   = transform;
    entity->name        = string_create(name);

    if (entity->parent)
        entity->context = entity->parent->context;
    else
        entity->context = (context) ? context : ecs->default_context;

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

static struct component_storage alloc_component(struct component_descriptor *descriptor,
                                                struct entity *entity)
{
    struct component_storage storage = {
        (descriptor->active_storage.data_size) ? list_alloc(&descriptor->active_storage) : 0,
        (descriptor->passive_storage.data_size) ? list_alloc(&descriptor->passive_storage) : 0,
    };

    struct component_reference ref = {
        descriptor,
        storage
    };

    list_push(&entity->components, &ref);

    return storage;
}

static void parse_properties_json(struct ecs_service *ecs,
                                  struct component_descriptor *descriptor,
                                  void *passive_storage,
                                  struct json_object *properties)
{
    list_for_each (struct json_element *, p_child, properties->children.values) {
        struct component_property *property = string_map_index(
            &descriptor->properties,
            (*p_child)->name.chars
        );

#ifdef DEBUG
        if (!property) {
            debug_log(
                SEVERITY_ERROR,
                "Property '%s' was not found for component '%s'.\n",
                (*p_child)->name.chars,
                descriptor->name.chars
            );

            abort();
        }
#endif // DEBUG

        property->serializer.deserializer(
            *p_child,
            (char *)passive_storage + property->offset,
            property->serializer.data
        );
    }
}

static void parse_component_json(struct ecs_service *ecs,
                                 struct entity *entity,
                                 struct json_object *component)
{
    struct json_string *component_name = json_index_object(component, "name");

    struct component_descriptor *descriptor = component_match_descriptor(
        ecs,
        component_name->string.chars
    );

#ifdef DEBUG
    if (!descriptor) {
        debug_log(
            SEVERITY_ERROR,
            "Component '%s' does not exist.\n",
            component_name->string.chars
        );

        abort();
    }
#endif // DEBUG

    struct component_storage instance = alloc_component(descriptor, entity);

    struct json_object *properties = json_index_object(component, "properties");
    if (properties)
        parse_properties_json(ecs, descriptor, instance.passive, properties);
} 

static struct entity *find_entity_by_index_recursive(struct entity *entity, int index, int *i)
{
    if (index == *i)
        return entity;

    ++(*i);

    list_for_each (struct entity *, p_child, entity->children) {
        struct entity *child = find_entity_by_index_recursive(*p_child, index, i);

        if (child)
            return child;
    }

    return 0;
}

static void parse_override_json(struct ecs_service *ecs,
                                struct entity *entity,
                                struct json_object *override)
{
    struct json_number *index_json = json_index_object(override, "index");
    struct json_string *name_json = json_index_object(override, "name");
    struct json_array *component_overrides = json_index_object(override, "components");

    int index = index_json->integer;
    int iter = 0;

    struct entity *to_override = find_entity_by_index_recursive(entity, index, &iter);

    if (name_json)
        entity_set_name(to_override, name_json->string.chars);

    list_for_each (struct json_object *, p_component_json, component_overrides->elements) {
        struct json_object *properties = json_index_object(*p_component_json, "properties");
        struct json_number *component_index_json = json_index_object(*p_component_json, "index");

        struct component_reference *component = list_index(
            &to_override->components,
            component_index_json->integer
        );

        parse_properties_json(ecs, component->descriptor, component->storage.passive, properties);
    }
}

static struct entity *parse_json(struct ecs_service *ecs,
                                 struct json_object *json,
                                 const char *name,
                                 struct context *context,
                                 struct entity *parent);

static struct entity *load_entity(struct ecs_service *ecs,
                                  const char *path,
                                  const char *name,
                                  struct context *context,
                                  struct entity *parent)
{
    char *json_string = file_to_buffer(path, 0);

#ifdef DEBUG
    if (!json_string) {
        debug_log(
            SEVERITY_ERROR,
            "Failed to load entity '%s', file could not be read.\n",
            path
        );

        abort();
    }
#endif // DEBUG

    struct json_object *json = json_parse_string(json_string);
    file_free_buffer(json_string);

    struct entity *entity = parse_json(ecs, json, name, context, parent);

    json_destroy_object(json);

    return entity;
}

static struct entity *parse_json(struct ecs_service *ecs,
                                 struct json_object *json,
                                 const char *name,
                                 struct context *context,
                                 struct entity *parent)
{
    struct json_string *name_json = json_index_object(json, "name");
    struct json_string *resource_path = json_index_object(json, "resource_path");
    struct json_array *components = json_index_object(json, "components");

    struct entity *entity = 0;

    if (resource_path) {
        struct json_array *overrides = json_index_object(json, "overrides");

        entity = load_entity(
            ecs,
            resource_path->string.chars,
            name ? name : name_json->string.chars,
            context,
            parent
        );

        if (overrides) {
            list_for_each (struct json_object *, p_override, overrides->elements) {
                parse_override_json(ecs, entity, *p_override);
            }
        }
    } else {
        entity = entity_create(ecs, name_json->string.chars, context, parent);

        struct json_array *children = json_index_object(json, "children");

        if (children) {
            list_for_each (struct json_object *, p_child, children->elements) {
                parse_json(ecs, *p_child, 0, context, entity);
            }
        }
    }

    if (components) {
        list_for_each (struct json_object *, p_component, components->elements) {
            parse_component_json(ecs, entity, *p_component);
        }
    }

    return entity;
}

static void initialize_components(struct entity *entity)
{
    list_for_each (struct component_reference, component, entity->components) {
        if (component->descriptor->init) {
            component->descriptor->init(
                entity,
                component->storage,
                component->descriptor->callback_data
            );
        }
    }

    list_for_each (struct entity *, p_child, entity->children) {
        initialize_components(*p_child);
    }
}

static void entered_tree(struct entity *entity)
{
    list_for_each (struct component_reference, component, entity->components) {
        if (component->descriptor->entered_tree) {
            component->descriptor->entered_tree(
                entity,
                component->storage,
                component->descriptor->callback_data
            );
        }
    }

    list_for_each (struct entity *, p_child, entity->children) {
        entered_tree(*p_child);
    }
}

struct entity *entity_load(struct ecs_service *ecs,
                           const char *path,
                           struct context *context,
                           struct entity *parent)
{
    struct entity *entity = load_entity(ecs, path, 0, context, parent);

    initialize_components(entity);
    entered_tree(entity);

    return entity;
}

void entity_set_name(struct entity *entity, const char *name)
{
    string_destroy(entity->name);
    entity->name = string_create(name);
}

struct entity *entity_find_child_recursive(struct entity *entity, const char *child)
{
    list_for_each (struct entity *, p_child, entity->children) {
        if (string_eq_ptr(child, (*p_child)->name.chars)) {
            return *p_child;
        } else {
            struct entity *entity = entity_find_child_recursive(*p_child, child);
            if (entity)
                return entity;
        }
    }

    return 0;
}

struct component_storage component_instance(struct ecs_service *ecs,
                                            struct entity *entity,
                                            const char *component)
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

    struct component_storage storage = alloc_component(descriptor, entity);

    if (descriptor->init)
        descriptor->init(entity, storage, descriptor->callback_data);

    if (descriptor->entered_tree)
        descriptor->entered_tree(entity, storage, descriptor->callback_data);

    return storage;
}

void component_destroy_instance(struct ecs_service *ecs,
                                struct entity *entity,
                                struct component_storage storage)
{
    list_for_each (struct component_reference, ref, entity->components) {
        if (ref->storage.active == storage.active &&
            ref->storage.passive == storage.passive) {
            destroy_component_instance(entity, *ref);
            list_remove(&entity->components, ref);

            return;
        }
    }
}

struct component_storage component_get_storage(struct ecs_service *ecs,
                                               struct entity *entity,
                                               const char *name)
{
    struct component_descriptor *descriptor = component_match_descriptor(ecs, name);

    list_for_each (struct component_reference, ref, entity->components) {
        if (ref->descriptor == descriptor)
            return ref->storage;
    }

    return (struct component_storage){ 0, 0 };
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

static void register_component_properties(struct ecs_service *ecs,
                                          struct component_descriptor *descriptor,
                                          struct component_registry_info *info)
{
    for (int i = 0; i < info->property_count; ++i) {
        struct property_serializer *serializer = string_map_index(
            &ecs->property_serializers,
            info->properties[i].type
        );

#ifdef DEBUG
        if (!serializer) {
            debug_log(
                SEVERITY_ERROR,
                "No serializer for property type '%s'\n",
                info->properties[i].type
            );

            abort();
        }
#endif // DEBUG

        struct component_property property = {
            .offset         = info->properties[i].offset,
            .serializer     = *serializer,
        };

        string_map_insert(&descriptor->properties, info->properties[i].name, &property);
    }
}

struct component_descriptor *component_register(struct ecs_service *ecs,
                                                struct component_registry_info *info)
{
    struct component_descriptor *descriptor = list_alloc(&ecs->components);

    if (info->passive_storage_size)
        list_init(&descriptor->passive_storage, info->passive_storage_size);

    if (info->active_storage_size)
        list_init(&descriptor->active_storage, info->active_storage_size);

    string_map_init(&descriptor->properties, sizeof(struct component_property));

    register_component_properties(ecs, descriptor, info);

    descriptor->name            = string_create(info->name);
    descriptor->callback_data   = info->callbacks.data;
    descriptor->init            = info->callbacks.init;
    descriptor->entered_tree    = info->callbacks.entered_tree;
    descriptor->cleanup         = info->callbacks.cleanup;

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