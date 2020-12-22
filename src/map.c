#include "peachcc.h"
Map *new_map(void)
{
    Map *map = malloc(sizeof(Map));
    map->keys = new_vec();
    map->vals = new_vec();
    return map;
}

void map_put(Map *map, char *key, void *val)
{
    vec_push(map->keys, key);
    vec_push(map->vals, val);
}

void map_puti(Map *map, char *key, int val)
{
    map_put(map, key, (void *)(intptr_t)val);
}

void *map_get(Map *map, char *key, size_t length)
{
    for (int i = map->keys->len - 1; i >= 0; i--)
        if (!strncmp(map->keys->data[i], key, length))
            return map->vals->data[i];
    return NULL;
}