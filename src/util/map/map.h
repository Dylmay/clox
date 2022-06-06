#ifndef __CLOX_UTIL_MAP_H__
#define __CLOX_UTIL_MAP_H__

#include <stdbool.h>
#include "list.h"

typedef uint32_t hash_t;
typedef hash_t (*hash_fn)(const void *a);

typedef struct __matcher {
	bool (*is_match)(const void *a, struct __matcher *matcher);
	hash_t hash;
} matcher_t;

struct map_entry {
	void *key;
	hash_t hash;
	bool tombstoned;
	uint8_t value[];
};

typedef struct {
	size_t cnt;
	size_t cap;
	size_t data_sz;
	struct map_entry *entries;
	hash_fn hash;
} hashmap_t;

#define MAP_MAX_LOAD 0.75

#define map_of_type(elem_type, fn) (map_new(sizeof(elem_type), fn))

static inline hashmap_t map_new(size_t val_sz, hash_fn hasher)
{
	return (hashmap_t){
		.cnt = 0,
		.cap = 0,
		.data_sz = val_sz,
		.entries = NULL,
		.hash = hasher,
	};
}

void map_adjust_capacity(hashmap_t *map, size_t cap);

static inline void map_free(hashmap_t *map)
{
	reallocate(map->entries,
		   (sizeof(struct map_entry) + map->data_sz) * map->cap, 0);
	map->cap = 0;
	map->cnt = 0;
	map->entries = NULL;
}

bool map_insert(hashmap_t *map, void *key, const void *value);
bool map_remove(hashmap_t *map, void *key);
void map_insert_all(const hashmap_t *from, hashmap_t *to);

void *map_get(const hashmap_t *map, const void *key);
void *map_find(const hashmap_t *map, matcher_t *matcher);

#endif // __CLOX_UTIL_MAP_H__