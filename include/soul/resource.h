#ifndef RESOURCE_H
#define RESOURCE_H

struct resource;

#include <stdlib.h>

#include "string.h"
#include "core.h"

typedef void(*resource_deallocator_t)(void *resource);

struct resource
{
    struct string           name;
    resource_deallocator_t  deallocator;
    void *                  p_user_data;
};

void *resource_create(struct soul_instance *instance,
                      const char *name,
                      size_t user_data_size,
                      resource_deallocator_t deallocator);
void *resource_get(struct soul_instance *instance, const char *name);

#endif // RESOURCE_H