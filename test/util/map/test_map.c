#include "test_map.h"

#include "util/map/map.h"
#include <assert.h>

struct value {
	int val;
};
hash_t hash_str(const void *a)
{
	const struct value *val = (struct value *)a;
	return val->val;
}

void map_test_all()
{
	map_test_new();
	map_test_insert();
	map_test_remove();
}

void map_test_new()
{
	hashmap_t map = map_of_type(struct value, &hash_str);

	map_free(&map);
}

void map_test_insert()
{
	hashmap_t map = map_of_type(struct value, &hash_str);
	struct value vals[] = {
		{
			.val = 0,
		},
		{
			.val = 1,
		},
		{
			.val = 2,
		},
		{
			.val = 3,
		},
	};

	struct map_entry_expected {
		void *key;
		hash_t hash;
		bool tombstoned;
		struct value val;
	};

	struct map_other_expected {
		void *key;
		hash_t hash;
		bool tombstoned;
		uint8_t vals[4];
	};

	struct map_un_expected {
		void *key;
		hash_t hash;
		bool tombstoned;
		uint8_t vals[1];
	};

	for (int i = 0; i < 3; i++) {
		map_insert(&map, &(vals[i]), &(vals[i]));
	}

	map_free(&map);
}

void map_test_remove()
{
	hashmap_t map = map_of_type(struct value, &hash_str);
	struct value vals[] = {
		{
			.val = 0,
		},
		{
			.val = 1,
		},
		{
			.val = 2,
		},
		{
			.val = 3,
		},
	};

	struct map_entry_expected {
		void *key;
		hash_t hash;
		bool tombstoned;
		struct value val;
	};

	struct map_other_expected {
		void *key;
		hash_t hash;
		bool tombstoned;
		uint8_t vals[4];
	};

	struct map_un_expected {
		void *key;
		hash_t hash;
		bool tombstoned;
		uint8_t vals[1];
	};

	for (int i = 0; i < 4; i++) {
		map_insert(&map, &(vals[i]), &(vals[i]));
	}

	for (int i = 0; i < 4; i++) {
		assert(map_remove(&map, &(vals[i])));
	}

	assert(("unexpected map size", map.cnt == 0));

	map_free(&map);
}

void map_test_get()
{
}