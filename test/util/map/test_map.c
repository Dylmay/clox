#include "test_map.h"

#include "debug/timer/timer.h"

#include "util/map/map.h"
#include <stdlib.h>
#include <stdio.h>
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
	map_test_get();
}

void map_bench_all()
{
	map_bench_insert();
	map_bench_traffic();
	map_bench_grow();
	map_bench_get();
	map_bench_find();
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

	for (int i = 0; i < 4; i++) {
		map_insert(&map, &(vals[i]), &(vals[i]));
	}

	for (int i = 0; i < 4; i++) {
		assert(map_remove(&map, &(vals[i])));
	}

	assert(("unexpected map size", map_size(&map) == 0));

	map_free(&map);
}

void map_test_get()
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

	for (int i = 0; i < 4; i++) {
		map_insert(&map, &(vals[i]), &(vals[i]));
	}

	for (int i = 0; i < 4; i++) {
		assert(("Values do not match",
			vals[i].val ==
				((struct value *)map_get(&map, &(vals[i])))
					->val));
	}

	assert(("unexpected map size", map.cnt != 0));

	map_free(&map);
}

void __map_bench_insert()
{
#define VAL_CNT 10000

	hashmap_t map = map_of_type(struct value, &hash_str);
	struct value *values = malloc(sizeof(struct value) * VAL_CNT);
	for (size_t i = 0; i < VAL_CNT; i++) {
		values[i] = (struct value){ .val = i };
	}

	size_t prev_cap = map.cap;
	for (size_t i = 0; i < VAL_CNT; i++) {
		map_insert(&map, &(values[i]), &(values[i]));

		if (map.cap != prev_cap) {
			for (int y = 0; y <= i; y++) {
				const struct value *retval =
					map_get(&map, &(values[y]));

				assert(("No matching value found",
					retval != NULL));

				assert(("Values do not match",
					(values[y]).val == retval->val));
			}
			prev_cap = map.cap;
		}
	}

	for (int i = 0; i < VAL_CNT; i++) {
		const struct value *retval = map_get(&map, &(values[i]));

		assert(("No matching value found", retval != NULL));

		assert(("Values do not match", (values[i]).val == retval->val));
	}

	assert(("unexpected map size", map.cnt != 0));

	free(values);
	map_free(&map);

#undef VAL_CNT
}

void map_bench_insert()
{
	struct timespec time =
		time_function_mean(&__map_bench_insert, BENCHMARK_ITERATIONS);

	puts("Mean time for bulk insertion:");
	print_time(time, true);
	puts("");
}

void __map_bench_traffic()
{
#define VAL_CNT 10000

	hashmap_t map = map_of_type(struct value, &hash_str);
	struct value *values = malloc(sizeof(struct value) * VAL_CNT);
	for (size_t i = 0; i < VAL_CNT; i++) {
		values[i] = (struct value){ .val = i };
	}

	size_t prev_cap = map.cap;
	size_t prev_i = 0;
	for (size_t i = 0; i < VAL_CNT; i++) {
		map_insert(&map, &(values[i]), &(values[i]));

		if (map.cap != prev_cap) {
			for (size_t y = prev_i; y <= i; y++) {
				assert(("Unable to find elem",
					map_remove(&map, &(values[y]))));
			}
			prev_cap = map.cap;
			prev_i = i + 1;
		}
	}

	assert(("unexpected map size", map.cnt != 0));

	free(values);
	map_free(&map);

#undef VAL_CNT
}

void map_bench_traffic()
{
	struct timespec time =
		time_function_mean(&__map_bench_traffic, BENCHMARK_ITERATIONS);

	puts("Mean time for traffic insertion:");
	print_time(time, true);
	puts("");
}
void map_bench_grow()
{
}
void map_bench_get()
{
}
void map_bench_find()
{
}
