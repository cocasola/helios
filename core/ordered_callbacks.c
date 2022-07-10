#include <soul/ordered_callbacks.h>

static struct callback_order create_callback_order(ordered_callback_t initial_callback,
                                                   int order, void *initial_callback_data)
{
    struct callback_order callback_order;

    list_init(&callback_order.callbacks, sizeof(struct ordered_callback));
    callback_order.order = order;

    struct ordered_callback ordered_callback = { initial_callback, initial_callback_data };

    struct ordered_callback *p_callback = list_push(
        &callback_order.callbacks,
        &ordered_callback
    );

    return callback_order;
}

ordered_callback_handle ordered_callbacks_insert(struct list *callback_orders,
                                                 ordered_callback_t callback,
                                                 int order, void *data, bool_t thread_safe)
{
    list_for_each (struct callback_order, callback_order, *callback_orders) {
        if (order < callback_order->order) {
            struct callback_order new_callback_order = create_callback_order(callback, order, data);
            list_insert_before(callback_orders, callback_order, &new_callback_order);

            return list_node_data_ptr(struct ordered_callback, new_callback_order.callbacks.head);
        }
        else if (order == callback_order->order) {
            struct ordered_callback ordered_callback = { callback, data };

            struct ordered_callback *p_ordered_callback = list_push(
                &callback_order->callbacks,
                &ordered_callback
            );

            return p_ordered_callback;
        }
    }

    struct callback_order callback_order = create_callback_order(callback, order, data);
    list_push(callback_orders, &callback_order);

    return list_node_data_ptr(struct ordered_callback, callback_order.callbacks.head);
}