#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "list.h"

typedef void(*callback_t)(void *data, void *user_data);

struct callback
{
    void *      data;
    callback_t  callback;
};

struct callback *   callbacks_subscribe(struct list *callbacks, callback_t callback, void *data);
void                callbacks_dispatch(struct list *callbacks, void *data);

#endif // CALLBACKS_H