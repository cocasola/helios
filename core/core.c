#include <soul/core.h>
#include <soul/services.h>

struct soul_instance *soul_init_instance(struct soul_instance_init_info *init_info)
{
    struct soul_instance *instance = calloc(1, sizeof(struct soul_instance));

    list_init(&instance->resources, sizeof(struct resource));
    list_init(&instance->callbacks, sizeof(struct callback_order));

    services_init_all(instance);

    return instance;
}

void soul_destroy_instance(struct soul_instance *instance)
{
    list_for_each (struct resource, resource, instance->resources) {
        resource->deallocator(resource->p_user_data);
        free(resource->p_user_data);
        string_destroy(resource->name);
    }

    list_destroy(&instance->resources);
}