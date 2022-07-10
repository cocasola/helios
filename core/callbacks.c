#include <soul/callbacks.h>

struct callback *callbacks_subscribe(struct list *callbacks, callback_t callback, void *data)
{
    struct callback callback_struct = {
        .callback   = callback,
        .data       = data
    };

    return list_push(callbacks, &callback_struct);
}

void callbacks_dispatch(struct list *callbacks, void *data)
{
    list_for_each (struct callback, callback, *callbacks) {
        callback->callback(data, callback->data);
    }
}