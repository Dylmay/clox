#ifndef __CLOX_UTIL_hashset_H__
#define __CLOX_UTIL_hashset_H__

#include "map.h"

/**
 * @brief hashset struct. Created using hashset_new(), contents must be freed after use by hashset_free()
 * @see hashset_new()
 * @see hashset_free()
 */
typedef hashmap_t hashset_t;

#define hashset_new(fn) (map_new(0, fn))
#define hashset_free(set) (map_free(set))
#define hashset_insert(set, key) (map_insert(set, key, NULL))
#define hashset_remove(set, key) (map_remove(set, key))
#define hashset_insert_all(into, from) (map_insert_all(into, from))
#define hashset_get(set, key) (map_get(set, key))
#define hashset_find(set, matcher) (map_find_by_key((set), (matcher)).key)

//! @brief set for each struct
struct hashset_for_each {
	void (*func)(void *key, struct hashset_for_each *data);
};

/**
 * @brief runs the given function within hashset_for_each on each entry
 *
 * @param set the hashset to process
 * @param for_each the for each function struct
 */
void hashset_for_each(hashset_t *set, struct hashset_for_each *for_each);

#endif // __CLOX_UTIL_hashset_H__