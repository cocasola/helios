#include <soul/resource.h>

void *resource_create(struct soul_instance *instance,
                      const char *name,
                      size_t user_data_size,
                      resource_deallocator_t deallocator)
{
    void *p_user_data = calloc(1, user_data_size);

    struct resource *resource = list_alloc(&instance->resources);

    resource->name          = string_create(name);
    resource->p_user_data   = p_user_data;
    resource->deallocator   = deallocator;

    return p_user_data;
}

void *resource_get(struct soul_instance *instance, const char *name)
{
    list_for_each (struct resource, resource, instance->resources) {
        if (string_eq_ptr(name, resource->name.chars))
            return resource->p_user_data;
    }

    return 0;
}