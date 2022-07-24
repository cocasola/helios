#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <string.h>

struct list_node_header
{
    struct list_node_header *   prev;
    struct list_node_header *   next;
};

struct list
{
    size_t                      data_size;
    struct list_node_header *   head;
    struct list_node_header *   tail;
};

#define list_node_data_ptr(type, node)  ((type *)(node + 1))
#define list_node_data(type, node)      (*(type *)(node + 1))
#define list_node_from_data(p_data)     (((struct list_node_header *)p_data) - 1)

#define list_for_each(iter_type, iter, list)                                    \
    for (iter_type *iter = (list).head ?                                        \
                          list_node_data_ptr(iter_type, (list).head) :          \
                          0;                                                    \
         iter;                                                                  \
         iter = list_node_from_data(iter)->next ?                               \
                list_node_data_ptr(iter_type, list_node_from_data(iter)->next) :\
                0)

void                        list_init(struct list *list, size_t data_size);
void                        list_destroy(struct list *list);
struct list_node_header *   list_alloc_node(struct list *list);
void *                      list_alloc(struct list *list);
void *                      list_push(struct list *list, void *p_data);
void                        list_remove(struct list *list, void *p_data);
void                        list_remove_value(struct list *list, void *p_data);
void *                      list_insert(struct list *list, void *p_after, void *p_data);
void *                      list_insert_before(struct list *list, void *p_before, void *p_data);
void *                      list_get_next(void *data);
void *                      list_get_head(struct list *list);

#endif // LIST_H