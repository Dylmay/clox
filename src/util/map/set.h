#ifndef __CLOX_UTIL_SET_H__
#define __CLOX_UTIL_SET_H__

#include "map.h"

typedef hashmap_t hashset_t;

#define SET_MAX_LOAD MAP_MAX_LOAD

#define set_new(fn) (map_new(sizeof(bool), fn))
#define set_free(set) (map_free(set))
#define set_insert(set, key) (map_insert(set, key, NULL))
#define set_remove(set, key) (map_remove(set, key))
#define set_insert_all(from, to) (map_insert_all(from, to))
#define set_get(set, key) (map_get(set, key))
#define set_get_by_hash(set, hash) (map_get_by_hash(set, hash))
#define set_find(set, matcher) (map_find(set, matcher))

#endif // __CLOX_UTIL_SET_H__