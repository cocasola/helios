#include <soul/string_map.h>
#include <soul/string.h>

#define DEFAULT_PAIR_LIST_COUNT 16

struct pair
{
    struct string   key;
    void *          value;
};

static int hash_string(const char *string, int max_value)
{
    int total = 0, i = 0;

    while (string[i] != '\0') {
        total += string[i];
        ++i;
    }

    return total % max_value;
}

void string_map_init(struct string_map *map, size_t data_size)
{
    list_init(&map->values, data_size);

    map->pairs      = malloc(sizeof(struct list)*DEFAULT_PAIR_LIST_COUNT);
    map->data_size  = data_size;

    for (int i = 0; i < DEFAULT_PAIR_LIST_COUNT; ++i) {
        list_init(map->pairs + i, sizeof(struct pair));
    }
}

void string_map_destroy(struct string_map *map)
{
    list_destroy(&map->values);

    for (int i = 0; i < DEFAULT_PAIR_LIST_COUNT; ++i) {
        list_for_each (struct pair, pair, map->pairs[i]) {
            string_destroy(pair->key);
        }

        list_destroy(map->pairs + i);
    }

    free(map->pairs);
}

void *string_map_insert(struct string_map *map, const char *key, void *value)
{
    void *allocation = string_map_alloc(map, key);
    memcpy(allocation, value, map->data_size);

    return allocation;
}

void *string_map_alloc(struct string_map *map, const char *key)
{
    void *data = list_alloc(&map->values);

    int hash = hash_string(key, DEFAULT_PAIR_LIST_COUNT);
    struct list *pair_list = map->pairs + hash;

    struct pair *pair = list_alloc(pair_list);
    pair->value = data;
    pair->key   = string_create(key);

    return data;
}

void string_map_remove(struct string_map *map, const char *key)
{
    int hash = hash_string(key, DEFAULT_PAIR_LIST_COUNT);
    struct list *pairs = map->pairs + hash;

    list_for_each (struct pair, pair, *pairs) {
        if (string_eq_ptr(key, pair->key.chars)) {
            list_remove(&map->values, pair->value);
            list_remove(pairs, pair);

            return;
        }
    }
}

void *string_map_index(struct string_map *map, const char *key)
{
    int hash = hash_string(key, DEFAULT_PAIR_LIST_COUNT);
    struct list *pairs = map->pairs + hash;

    list_for_each (struct pair, pair, *pairs) {
        if (string_eq_ptr(key, pair->key.chars))
            return pair->value;
    } 

    return 0;
}