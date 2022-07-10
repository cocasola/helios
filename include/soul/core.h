#ifndef CORE_H
#define CORE_H

struct soul_instance;

#include "resource.h"
#include "list.h"
#include "ordered_callbacks.h"

struct soul_instance
{
    struct list resources; // struct resource
    struct list callbacks; // struct callback_order
};

struct soul_instance_init_info
{

};

struct soul_instance *soul_init_instance(struct soul_instance_init_info *init_info);
void soul_destroy_instance(struct soul_instance *instance);

#endif // CORE_H