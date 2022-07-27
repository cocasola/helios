#include <soul/core.h>
#include <soul/services.h>
#include <soul/components.h>
#include <soul/ecs.h>
#include <soul/property_serialization.h>

struct soul_instance *soul_init_instance(struct soul_instance_init_info *init_info)
{
    struct soul_instance *instance = calloc(1, sizeof(struct soul_instance));

    list_init(&instance->resources, sizeof(struct resource));
    list_init(&instance->callbacks, sizeof(struct callback_order));

    services_create_all(instance);

    struct ecs_service *ecs = resource_get(instance, ECS_SERVICE);
    property_serialization_populate_table(instance, &ecs->property_serializers);

    components_register_all(instance);

    return instance;
}

void soul_destroy_instance(struct soul_instance *instance)
{
    list_for_each (struct resource, resource, instance->resources) {
        if (resource->deallocator)
            resource->deallocator(resource->p_user_data);

        free(resource->p_user_data);
        string_destroy(resource->name);
    }

    list_destroy(&instance->resources);
}