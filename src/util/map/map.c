#include "map.h"

#include <string.h>

#define SIZEOF_ENTRY(map) (sizeof(struct map_entry) + map->data_sz)
#define ENTRY_AT(map, idx) (map->entries + (SIZEOF_ENTRY(map) * idx))

static struct map_entry *__map_find(const hashmap_t *map, const void *key,
				    hash_t hash);
static void __map_rebuild(hashmap_t *map, size_t cap);

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
	bucket->tombstoned = false;
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
		size_t offset = sizeof(struct map_entry) + map->data_sz;
		size_t idx_off = offset * idx;
		struct map_entry *entry = map->entries + idx_off;

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

		if (entry->key) {
			map_insert(to, entry->key, entry->value);
		}
	}
}

bool map_remove(hashmap_t *map, void *key)
{
	if (!map->cnt) {
		return false;
	}

	struct map_entry *entry = __map_find(map, key, map->hash(key));

	if (entry->key == NULL) {
		return false;
	}

	entry->key = NULL;
	entry->tombstoned = true;
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

static void __map_rebuild(hashmap_t *map, size_t new_cap)
{
	size_t cnt = 0;

	struct map_entry *entries = (struct map_entry *)reallocate(
		NULL, 0, SIZEOF_ENTRY(map) * new_cap);

	memset(entries, 0, SIZEOF_ENTRY(map) * new_cap);

	for (size_t i = 0; i < map->cap; i++) {
		struct map_entry *entry = ENTRY_AT(map, i);

		if (entry->key == NULL) {
			continue;
		}

		struct map_entry *dest =
			__map_find(map, entry->key, entry->hash);
		dest->key = entry->key;
		memcpy(dest->value, entry->value, map->data_sz);
		cnt++;
	}

	map_free(map);
	map->cap = new_cap;
	map->cnt = cnt;
	map->entries = entries;
}
