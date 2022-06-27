#ifndef __CLOX_UTIL_MAP_H__
#define __CLOX_UTIL_MAP_H__

#include <stdbool.h>
#include "util/list/list.h"
#include "hash.h"

struct __map_entry {
	void *key;
	hash_t hash;
	bool tombstoned;
	uint8_t value[];
};

typedef struct {
	size_t cnt;
	size_t tomb_cnt;
	size_t cap;
	size_t data_sz;
	uint8_t *entries;
	hash_fn hash;
} hashmap_t;

typedef struct {
	void *key;
	void *value;
} map_entry_t;

typedef struct __key_matcher {
	bool (*is_match)(const void *key, struct __key_matcher *matcher);
	hash_t hash;
} key_matcher_t;

typedef struct __val_matcher {
	bool (*is_match)(const void *value, struct __val_matcher *matcher);
} val_matcher_t;

typedef struct __for_each_e {
	void (*func)(map_entry_t entry, struct __for_each_e *data);
} for_each_entry_t;

#define MAP_MAX_LOAD 0.75

#define map_of_type(elem_type, fn) (map_new(sizeof(elem_type), fn))

static inline hashmap_t map_new(size_t val_sz, hash_fn hasher)
{
	return (hashmap_t){
		.cnt = 0,
		.tomb_cnt = 0,
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
		   (sizeof(struct __map_entry) + map->data_sz) * map->cap, 0);
	map->cap = 0;
	map->cnt = 0;
	map->tomb_cnt = 0;
	map->entries = NULL;
}

static inline size_t map_size(const hashmap_t *map)
{
	return map->cnt - map->tomb_cnt;
}

bool map_insert(hashmap_t *map, void *key, const void *value);
bool map_remove(hashmap_t *map, const void *key);
bool map_set(hashmap_t *map, const void *key, const void *value);
void map_insert_all(const hashmap_t *from, hashmap_t *to);

void *map_get(const hashmap_t *map, const void *key);
map_entry_t map_find_by_key(const hashmap_t *map, key_matcher_t *matcher);
map_entry_t map_find_by_value(const hashmap_t *map, val_matcher_t *matcher);

void map_entries_for_each(hashmap_t *map, for_each_entry_t *for_each);

#endif // __CLOX_UTIL_MAP_H__