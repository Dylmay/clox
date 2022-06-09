#include "map.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define SIZEOF_ENTRY(map) (sizeof(struct map_entry) + map->data_sz)
#define ENTRY_AT(map, idx)                                                     \
	((struct map_entry *)(map->entries + (SIZEOF_ENTRY(map) * idx)))

static struct map_entry *__map_find(const hashmap_t *map, const void *key,
				    hash_t hash);
static void __map_rebuild(hashmap_t *map, size_t cap);
static void __tombstone_entry(hashmap_t *map, struct map_entry *entry);
static void __revive_entry(hashmap_t *map, struct map_entry *entry);

bool map_insert(hashmap_t *map, void *key, const void *value)
{
	if (map->cnt + 1 > map->cap * MAP_MAX_LOAD) {
		size_t capacity = GROW_CAPACITY(map->cap);
		map_adjust_capacity(map, capacity);
	}

	hash_t hash = map->hash(key);

	struct map_entry *bucket = __map_find(map, key, hash);

	if (!bucket->key && !bucket->tombstoned) {
		map->cnt++;
	}

	bucket->key = key;
	if (bucket->tombstoned) {
		__revive_entry(map, bucket);
	}
	bucket->hash = hash;
	if (value) {
		memcpy(bucket->value, value, map->data_sz);
	} else {
		memset(bucket->value, 0, map->data_sz);
	}

	return true;
}

static struct map_entry *__map_find(const hashmap_t *map, const void *key,
				    hash_t hash)
{
	uint32_t idx = hash % map->cap;
	struct map_entry *tombstone = NULL;

	while (true) {
		struct map_entry *entry = ENTRY_AT(map, idx);

		if (!entry->key) {
			if (!entry->tombstoned) {
				return tombstone ? tombstone : entry;
			}

			if (!tombstone) {
				tombstone = entry;
			}
		}

		if (entry->key == key) {
			return entry;
		}

		idx = (idx + 1) % map->cap;
	}
}

void map_adjust_capacity(hashmap_t *map, size_t cap)
{
	__map_rebuild(map, cap);
}

void map_insert_all(const hashmap_t *from, hashmap_t *to)
{
	for (size_t i = 0; i < from->cap; i++) {
		const struct map_entry *entry = ENTRY_AT(from, i);

		if (entry->key && !entry->tombstoned) {
			map_insert(to, entry->key, entry->value);
		}
	}
}

bool map_remove(hashmap_t *map, const void *key)
{
	if (!map->cnt) {
		return false;
	}

	struct map_entry *entry = __map_find(map, key, map->hash(key));

	if (entry->key == NULL) {
		return false;
	}

	__tombstone_entry(map, entry);
	return true;
}

void *map_get(const hashmap_t *map, const void *key)
{
	if (!map->cnt) {
		return NULL;
	}

	struct map_entry *entry = __map_find(map, key, map->hash(key));

	if (entry->key) {
		return entry->value;
	}

	return NULL;
}

void *map_find(const hashmap_t *map, matcher_t *matcher)
{
	if (!map->cnt) {
		return NULL;
	}

	hash_t idx = matcher->hash % map->cap;
	while (true) {
		struct map_entry *entry = ENTRY_AT(map, idx);

		if (!entry->key) {
			if (!entry->tombstoned) {
				return NULL;
			}
		} else if (entry->hash == matcher->hash &&
			   matcher->is_match(entry->key, matcher)) {
			return entry->key;
		}

		idx = (idx + 1) % map->cap;
	}
}

void map_keys_for_each(hashmap_t *map, void (*for_each)(void *key))
{
	for (size_t i = 0; i < map->cap; i++) {
		struct map_entry *entry = ENTRY_AT(map, i);

		if (entry->key == NULL || entry->tombstoned) {
			continue;
		}

		for_each(entry->key);
	}
}

static void __map_rebuild(hashmap_t *map, size_t new_cap)
{
	hashmap_t new_map = map_new(map->data_sz, map->hash);
	new_map.cap = new_cap;

	new_map.entries = reallocate(NULL, 0, SIZEOF_ENTRY(map) * new_map.cap);
	memset(new_map.entries, 0, SIZEOF_ENTRY(map) * new_map.cap);

	map_insert_all(map, &new_map);
	free(map->entries);
	memcpy(map, &new_map, sizeof(hashmap_t));
}

static void __tombstone_entry(hashmap_t *map, struct map_entry *entry)
{
	assert(("entry must be alive before being tombstoned",
		entry->tombstoned == false));

	map->tomb_cnt++;
	entry->tombstoned = true;
	entry->key = NULL;
}

static void __revive_entry(hashmap_t *map, struct map_entry *entry)
{
	assert(("no tombs found within map", map->tomb_cnt != 0));
	assert(("entry must be tombstoned before revival",
		entry->tombstoned == true));

	map->tomb_cnt--;
	entry->tombstoned = false;
}