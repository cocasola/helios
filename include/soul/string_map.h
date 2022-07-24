#ifndef STRING_MAP_H
#define STRING_MAP_H

#include "list.h"

struct string_map
{
    struct list     values; // void
    struct list *   pairs; // struct pair
    size_t          data_size;
};

void    string_map_init(struct string_map *map, size_t data_size);
void    string_map_destroy(struct string_map *map);
void *  string_map_insert(struct string_map *map, const char *key, void *data);
void *  string_map_alloc(struct string_map *map, const char *key);
void    string_map_remove(struct string_map *map, const char *key);
void *  string_map_index(struct string_map *map, const char *key);

#endif // STRING_MAP_H