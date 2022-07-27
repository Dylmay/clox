#ifndef __CLOX_UTIL_MAP_H__
#define __CLOX_UTIL_MAP_H__

#include <stdbool.h>
#include "util/list/list.h"
#include "hash.h"

/**
 * @brief hashmap struct. Created using map_new(), contents must be freed after use by map_free()
 * @see map_new()
 * @see map_free()
 */
typedef struct {
	size_t cnt;
	size_t tomb_cnt;
	size_t cap;
	size_t data_sz;
	uint8_t *entries;
	hash_fn hash;
} hashmap_t;

//! @brief map entry struct
struct map_entry {
	void *key;
	void *value;
};

/**
 * @brief key matcher struct for finding map entries by key
 * @see map_find_by_key()
 */
struct key_matcher {
	bool (*is_match)(const void *key, struct key_matcher *matcher);
	hash_t hash;
};

/**
 * @brief value matcher struct for finding map entries by value
 * @see map_find_by_value()
 */
struct val_matcher {
	bool (*is_match)(const void *value, struct val_matcher *matcher);
};

/**
 * @brief for each entry struct for running a function on every entry
 * @see map_entries_for_each()
 *
 */
struct map_for_each_entry {
	void (*func)(struct map_entry entry, struct map_for_each_entry *data);
};

// max map load possible before the table should grow
#define MAP_MAX_LOAD 0.75

//! \def map_of_type(elem_type, fn) @brief Creates a new map with the given type
#define map_of_type(elem_type, fn) (map_new(sizeof(elem_type), fn))

/**
 * @brief creates a new map with the given value size and hash function
 *
 * @param val_sz the size of value entries
 * @param hasher key hash function
 * @return hashmap_t the new hashmap
 */
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

/**
 * @brief frees the passed values and related objects of the map.
 * Note: entry keys are not freed
 *
 * @param map the map to free
 */
void map_free(hashmap_t *map);

/**
 * @brief gets the size of the given map
 *
 * @param map the map to get the size of
 * @return size_t the map size
 */
static inline size_t map_size(const hashmap_t *map)
{
	return map->cnt - map->tomb_cnt;
}

/**
 * @brief inserts a new key/value pair from the given map. If the key is already present, the value is over-written
 *
 * @param map the map to insert within
 * @param key the key
 * @param value the value
 * @return true the insert succeeded
 * @return false the insert failed due to the key already being present
 */
bool map_insert(hashmap_t *map, void *key, const void *value);

/**
 * @brief removes a key/value pair from the given map
 *
 * @param map the map to remove within
 * @param key the key
 * @return true the remove succeeded
 * @return false the remove failed due to the key not being found
 */
bool map_remove(hashmap_t *map, const void *key);

/**
 * @brief
 *
 * @param map
 * @param key
 * @param value
 * @return true
 * @return false
 */
bool map_set(hashmap_t *map, const void *key, const void *value);

/**
 * @brief Inserts all values from the 'to' hashmap in to the 'from' hashmap.
 * Any keys that are already set within 'from' are skipped
 *
 * @param into the hashmap to insert into
 * @param from the hashmap to read from
 */
void map_insert_all(hashmap_t *into, const hashmap_t *from);

/**
 * @brief gets the value related to the passed key
 *
 * @param map the hashmap
 * @param key the key
 * @return void* the value assigned to the key. NULL if none is found
 */
void *map_get(const hashmap_t *map, const void *key);

/**
 * @brief finds an entry mathing by key
 *
 * @param map the hashmap to search
 * @param matcher the the key matcher to validate the passed key
 * @return struct map_entry the found entry. entry.key will be NULL if no match is found
 */
struct map_entry map_find_by_key(const hashmap_t *map,
				 struct key_matcher *matcher);

/**
 * @brief finds an entry matching by value
 *
 * @param map the hashmap to search
 * @param matcher the value matcher to validate the passed key
 * @return struct map_entry the found entyr. entry.key will be NULL if no match is found
 */
struct map_entry map_find_by_value(const hashmap_t *map,
				   struct val_matcher *matcher);

/**
 * @brief runs the given function within map_for_each_entry on each map entry
 *
 * @param map the hashmap to process
 * @param for_each the for each function struct
 */
void map_entries_for_each(hashmap_t *map, struct map_for_each_entry *for_each);

#endif // __CLOX_UTIL_MAP_H__