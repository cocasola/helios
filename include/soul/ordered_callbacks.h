#ifndef ORDERED_CALLBACKS_H
#define ORDERED_CALLBACKS_H

#include "list.h"
#include "typedefs.h"

typedef void(*ordered_callback_t)(void *data);

struct ordered_callback
{
    ordered_callback_t  fn_ptr;
    void *              data;
};

typedef struct ordered_callback *ordered_callback_handle;

struct callback_order
{
    int         order;
    struct list callbacks; // struct ordered_callback
};

ordered_callback_handle ordered_callbacks_insert(struct list *callback_orders,
                                                 ordered_callback_t callback,
                                                 int order,
                                                 void *data,
                                                 bool_t thread_safe);

#endif // ORDERED_CALLBACKS_H