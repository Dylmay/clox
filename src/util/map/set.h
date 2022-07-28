/**
 * @file set.h
 * @author Dylan Mayor
 * @brief header file for hashset implementation
 *
 */
#ifndef __CLOX_UTIL_hashset_H__
#define __CLOX_UTIL_hashset_H__

#include "map.h"

/**
 * @brief hashset struct. Created using hashset_new(). contents must be freed after use by hashset_free()
 * @see hashset_new()
 * @see hashset_free()
 */
typedef hashmap_t hashset_t;

/**
 *  @brief creates a new hash set using the given hashing function
 *
 * @param fn the hashing function
 * @return hashset_t the new set
 */
#define hashset_new(fn) (map_new(0, (fn)))

/**
 * @brief frees the passed hashset
 *
 * @param set the set to free
 *
 */
#define hashset_free(set) (map_free((set)))

/**
 * @brief inserts a new value in to the hash set
 *
 * @param set the set to insert in to
 * @param value the value to add. The pointer is not handled by the set, only used as a reference
 *
 * @return true the insert succeeded
 * @return false the insert failed due to the key already being present
 */
#define hashset_insert(set, value) (map_insert((set), (value), NULL))

/**
 * @brief removes a value from the hash set
 *
 * @param set the set to remove from
 * @param value the value to remove. The pointer is not handled by the set, only used as a reference
 *
 * @return true the remove succeeded
 * @return false the remove failed due to the key not being found
 *
 */
#define hashset_remove(set, value) (map_remove((set), (value)))

/**
 * @brief inserta all values from the 'from' hash set in to the 'into' hash set
 *
 * @param into the hash set to insert in to
 * @param from the hash set to insert from
 */
#define hashset_insert_all(into, from) (map_insert_all((into), (from)))

/**
 * @brief checks whether the given value is present in the set
 *
 * @param set the set to search
 * @param value the value to lookup
 *
 * @return true the value was found within the map
 * @return false the value was not found within the map
 */
#define hashset_has(set, value) (map_has((set), (value)))

/**
 * @brief finds an entry matching by key
 *
 * @param map the hashmap to search
 * @param matcher the the key matcher to validate the passed key
 * @return struct map_entry the found entry. entry.key will be NULL if no match is found
 */
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