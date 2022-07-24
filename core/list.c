#include <soul/list.h>

void list_init(struct list *list, size_t data_size)
{
    list->data_size = data_size;
    list->head      = 0;
    list->tail      = 0;
}

void list_destroy(struct list *list)
{
    struct list_node_header *node = list->head;

    while (node) {
        struct list_node_header *next = node->next;
        free(node);

        node = next;
    }
}

struct list_node_header *list_alloc_node(struct list *list)
{
    size_t size = sizeof(struct list_node_header) + list->data_size;
    struct list_node_header *node = (struct list_node_header *)calloc(1, size);

    return node;
}

void *list_alloc(struct list *list)
{
    struct list_node_header *node = list_alloc_node(list);

    if (list->tail) {
        list->tail->next    = node;
        node->prev          = list->tail;
        list->tail          = node;
    } else {
        list->head  = node;
        list->tail  = node;
    }

    return list_node_data_ptr(void, node);
}

void *list_push(struct list *list, void *p_data)
{
    void *p_new_data = list_alloc(list);
    memcpy(p_new_data, p_data, list->data_size);

    return p_new_data;
}

void list_remove(struct list *list, void *p_data)
{
    struct list_node_header *node = list_node_from_data(p_data);

    if (node->prev)
        node->prev->next = node->next;

    if (node->next)
        node->next->prev = node->prev;

    if (node == list->head)
        list->head = node->next;

    if (node == list->tail)
        list->tail = node->prev;

    free(node);
}

void list_remove_value(struct list *list, void *p_data)
{
    list_for_each (void, iter, *list) {
        if (memcmp(iter, p_data, list->data_size) == 0) {
            list_remove(list, iter);
            return;
        }
    }
}

void *list_insert(struct list *list, void *p_after, void *p_data)
{
    struct list_node_header *node = list_alloc_node(list);
    struct list_node_header *after_node = list_node_from_data(p_after);

    node->next = after_node->next;
    node->prev = after_node;

    after_node->next = node;

    if (node->next)
        node->next->prev = node;

    if (list->tail == after_node)
        list->tail = node;

    void *p_new_data = list_node_data_ptr(void, node);
    memcpy(p_new_data, p_data, list->data_size);

    return p_new_data;
}

void *list_insert_before(struct list *list, void *p_before, void *p_data)
{
    struct list_node_header *node = list_alloc_node(list);
    struct list_node_header *before_node = list_node_from_data(p_before);

    node->next = before_node;
    node->prev = before_node->prev;

    before_node->prev = node;

    if (node->prev)
        node->prev->next = node;

    if (list->head == before_node)
        list->head = node;

    void *p_new_data = list_node_data_ptr(void, node);
    memcpy(p_new_data, p_data, list->data_size);

    return p_new_data;
}

void *list_get_next(void *data)
{
    struct list_node_header *const node = list_node_from_data(data);

    return (node->next) ? list_node_data_ptr(void, node->next) : 0;
}

void *list_get_head(struct list *list)
{
    return list_node_data_ptr(void, list->head);
}