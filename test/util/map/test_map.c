#include "test_map.h"

#include "debug/timer/timer.h"

#include "util/map/map.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct value {
	int val;
};

struct val_matcher {
	struct key_matcher matcher;
	int match_val;
};

hash_t hash_val(const void *a)
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
	map_test_find();
}

void map_bench_all()
{
	map_bench_insert();
	map_bench_remove();
	map_bench_get();
	map_bench_find();
	map_bench_traffic();
}

void map_test_new()
{
	hashmap_t map = map_of_type(struct value, &hash_val);

	map_free(&map);
}

void map_test_insert()
{
	hashmap_t map = map_of_type(struct value, &hash_val);
	struct value *values = malloc(sizeof(struct value) * ITEM_CNT);
	for (size_t i = 0; i < ITEM_CNT; i++) {
		values[i] = (struct value){ .val = i };
	}

	// full insertion
	size_t prev_cap = map.cap;
	for (size_t i = 0; i < ITEM_CNT; i++) {
		assert(("Unable to insert item",
			map_insert(&map, &(values[i]), &(values[i]))));

		if (map.cap != prev_cap) {
			// Verify currently held items within the map
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

	// Verify all items within the map
	for (int i = 0; i < ITEM_CNT; i++) {
		const struct value *retval = map_get(&map, &(values[i]));

		assert(("No matching value found", retval != NULL));

		assert(("Values do not match", (values[i]).val == retval->val));
	}

	assert(("unexpected map size", map.cnt != 0));
	map_free(&map);

	// random insertion
	list_t chosen_rands = list_of_type(int);
	for (size_t i = 0; i < ITEM_CNT / 4; i++) {
		size_t rand_val;

		while (true) {
			rand_val = rand() % ITEM_CNT;
			bool new_rand = true;

			for (size_t y = 0; y < chosen_rands.cnt; y++) {
				if (rand_val ==
				    *((int *)list_get(&chosen_rands, y))) {
					new_rand = false;
					break;
				}
			}

			if (new_rand) {
				break;
			}
		}

		list_push(&chosen_rands, &rand_val);
		assert(("Unable to insert item",
			map_insert(&map, &(values[rand_val]),
				   &(values[rand_val]))));
	}
	for (size_t i = 0; i < chosen_rands.cnt; i++) {
		int idx = *((int *)list_get(&chosen_rands, i));
		const struct value *retval = map_get(&map, &(values[idx]));

		assert(("No matching value found", retval != NULL));
		assert(("Values do not match",
			(values[idx]).val == retval->val));
	}

	assert(("unexpected map size", map_size(&map) == ITEM_CNT / 4));

	hashmap_t map_two = map_of_type(struct value, &hash_val);
	struct value val_one = (struct value){ .val = 10 };
	struct value val_two = (struct value){ .val = 100 };

	map_insert(&map_two, &val_one, &val_one);
	map_insert(&map_two, &val_two, &val_two);

	assert(("unexpected map size", map_size(&map_two) == 2));
	assert(("could not find value", map_get(&map_two, &val_one) != NULL));
	assert(("unexpected value",
		val_one.val ==
			((const struct value *)map_get(&map_two, &val_one))
				->val));
	assert(("could not find value", map_get(&map_two, &val_two) != NULL));
	assert(("unexpected value",
		val_two.val ==
			((const struct value *)map_get(&map_two, &val_two))
				->val));

	free(values);
	map_free(&map);
	map_free(&map_two);
	list_free(&chosen_rands);
}

void map_test_remove()
{
	// setup
	hashmap_t map = map_of_type(struct value, &hash_val);
	struct value *values = malloc(sizeof(struct value) * ITEM_CNT);
	for (size_t i = 0; i < ITEM_CNT; i++) {
		values[i] = (struct value){ .val = i };
	}
	for (int i = 0; i < ITEM_CNT; i++) {
		map_insert(&map, &(values[i]), &(values[i]));
	}

	// all deletion
	for (int i = 0; i < ITEM_CNT; i++) {
		assert(("Unable to find expected item",
			map_remove(&map, &(values[i]))));
	}
	assert(("unexpected map size", map_size(&map) == 0));

	// reset map
	for (size_t i = 0; i < ITEM_CNT; i++) {
		map_insert(&map, &(values[i]), &(values[i]));
	}

	// random deletion
	list_t chosen_rands = list_of_type(int);
	for (size_t i = 0; i < ITEM_CNT / 4; i++) {
		size_t rand_val;

		while (true) {
			rand_val = rand() % ITEM_CNT;
			bool new_rand = true;

			for (size_t y = 0; y < chosen_rands.cnt; y++) {
				if (rand_val ==
				    *((int *)list_get(&chosen_rands, y))) {
					new_rand = false;
					break;
				}
			}

			if (new_rand) {
				break;
			}
		}

		list_push(&chosen_rands, &rand_val);
		assert(("Unable to find expected item",
			map_remove(&map, &(values[rand_val]))));
	}

	assert(("unexpected map size",
		map_size(&map) == ITEM_CNT - (ITEM_CNT / 4)));

	free(values);
	map_free(&map);
	list_free(&chosen_rands);
}

void map_test_get()
{
	hashmap_t map = map_of_type(struct value, &hash_val);
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

bool match_on_num(const void *entry, struct key_matcher *matcher)
{
	const struct value *val = (struct value *)entry;
	const struct val_matcher *val_m = (struct val_matcher *)matcher;

	return val->val == val_m->match_val;
}

void map_test_find()
{
	hashmap_t map = map_of_type(struct value, &hash_val);
	struct value *vals = malloc(sizeof(struct value) * ITEM_CNT);
	for (size_t i = 0; i < ITEM_CNT; i++) {
		vals[i] = (struct value){ .val = i };
	}

	for (size_t i = 0; i < ITEM_CNT; i++) {
		map_insert(&map, &(vals[i]), &(vals[i]));
	}

	for (size_t i = 0; i < ITEM_CNT; i++) {
		struct val_matcher matcher = (struct val_matcher) {
			.matcher = {
				.hash = hash_val(&(vals[i])),
				.is_match = &match_on_num,
			},
			.match_val = i,
		};

		const struct value *val =
			map_find(&map, (struct key_matcher *)&matcher);

		assert(("Unable to find value", val != NULL));
		assert(("Retrieved value does not match",
			vals[i].val == val->val));
	}

	list_t chosen_rands = list_of_type(int);
	for (size_t i = 0; i < ITEM_CNT / 4; i++) {
		size_t rand_val;

		while (true) {
			rand_val = rand() % ITEM_CNT;
			bool new_rand = true;

			for (size_t y = 0; y < chosen_rands.cnt; y++) {
				if (rand_val ==
				    *((int *)list_get(&chosen_rands, y))) {
					new_rand = false;
					break;
				}
			}

			if (new_rand) {
				break;
			}
		}

		list_push(&chosen_rands, &rand_val);
		assert(("Unable to find expected item",
			map_remove(&map, &(vals[rand_val]))));
	}

	for (size_t i = 0; i < ITEM_CNT / 8; i++) {
		size_t rand_val;

		while (true) {
			rand_val = rand() % ITEM_CNT;
			bool found_rand = false;

			for (size_t y = 0; y < chosen_rands.cnt; y++) {
				if (rand_val ==
				    *((int *)list_get(&chosen_rands, y))) {
					found_rand = true;
					break;
				}
			}

			if (!found_rand) {
				break;
			}
		}

		struct val_matcher matcher = (struct val_matcher) {
			.matcher = {
				.hash = hash_val(&(vals[rand_val])),
				.is_match = &match_on_num,
			},
			.match_val = rand_val,
		};

		const struct value *val =
			map_find(&map, (struct key_matcher *)&matcher);

		assert(("Unable to find value", val != NULL));
		assert(("Retrieved value does not match",
			vals[rand_val].val == val->val));
	}

	list_free(&chosen_rands);
	map_free(&map);
	free(vals);
}

struct timespec __map_bench_insert()
{
	struct timespec timer;

	hashmap_t map = map_of_type(struct value, &hash_val);
	struct value value = (struct value){ .val = 1 };

	timer_start(&timer);
	map_insert(&map, &value, &value);
	timer = timer_end(timer);

	map_free(&map);

	return timer;
}

struct timespec __map_bench_insert_rand()
{
	struct timespec timer;

	int rand_val = rand() % ITEM_CNT;
	struct value value = (struct value){ .val = rand_val };
	hashmap_t map = map_of_type(struct value, &hash_val);

	timer_start(&timer);
	map_insert(&map, &value, &value);
	timer = timer_end(timer);

	map_free(&map);

	return timer;
}

struct timespec __map_bench_insert_clash()
{
	struct timespec timer;

	int rand_val = rand() % ITEM_CNT;
	struct value val_one = (struct value){ .val = rand_val };
	struct value val_two = (struct value){ .val = rand_val * 10 };
	hashmap_t map = map_of_type(struct value, &hash_val);

	map_insert(&map, &val_one, &val_one);

	timer_start(&timer);
	map_insert(&map, &val_two, &val_two);
	timer = timer_end(timer);

	map_free(&map);

	return timer;
}

void map_bench_insert()
{
	struct timespec avg_time_i = __map_bench_insert();
	struct timespec avg_time_ir = __map_bench_insert_rand();
	struct timespec avg_time_ic = __map_bench_insert_clash();

	for (size_t iter = 1; iter < BENCHMARK_ITERATIONS; iter++) {
		struct timespec time_taken = __map_bench_insert();
		avg_time_i = timespec_avg(avg_time_i, time_taken);

		time_taken = __map_bench_insert_rand();
		avg_time_ir = timespec_avg(avg_time_ir, time_taken);

		time_taken = __map_bench_insert_clash();
		avg_time_ic = timespec_avg(avg_time_ic, time_taken);
	}

	puts("Mean time for single insertion:");
	timespec_print(avg_time_i, true);
	puts("");
	puts("Mean time for single insertion rand:");
	timespec_print(avg_time_ir, true);
	puts("");
	puts("Mean time for single insertion clash:");
	timespec_print(avg_time_ic, true);
	puts("");
}

struct timespec __map_bench_remove()
{
	struct timespec timer;

	hashmap_t map = map_of_type(struct value, &hash_val);
	struct value value = (struct value){ .val = 0 };

	timer_start(&timer);
	map_remove(&map, &value);
	timer = timer_end(timer);

	map_free(&map);

	return timer;
}

struct timespec __map_bench_remove_rand()
{
	struct timespec timer;

	int rand_val = rand() % ITEM_CNT;
	struct value value = (struct value){ .val = rand_val };
	hashmap_t map = map_of_type(struct value, &hash_val);

	map_insert(&map, &value, &value);

	timer_start(&timer);
	map_remove(&map, &value);
	timer = timer_end(timer);

	map_free(&map);

	return timer;
}

struct timespec __map_bench_remove_clash()
{
	struct timespec timer;

	int rand_val = rand() % ITEM_CNT;
	struct value val_one = (struct value){ .val = rand_val };
	struct value val_two = (struct value){ .val = rand_val * 10 };
	hashmap_t map = map_of_type(struct value, &hash_val);

	map_insert(&map, &val_one, &val_one);
	map_insert(&map, &val_two, &val_two);

	timer_start(&timer);
	map_remove(&map, &val_two);
	timer = timer_end(timer);

	map_free(&map);

	return timer;
}

void map_bench_remove()
{
	struct timespec avg_time_r = __map_bench_remove();
	struct timespec avg_time_rc = __map_bench_remove_clash();

	for (size_t iter = 1; iter < BENCHMARK_ITERATIONS; iter++) {
		struct timespec time_taken = __map_bench_remove();
		avg_time_r = timespec_avg(avg_time_r, time_taken);

		time_taken = __map_bench_remove_clash();
		avg_time_rc = timespec_avg(avg_time_rc, time_taken);
	}

	puts("Mean time for single removal:");
	timespec_print(avg_time_r, true);
	puts("");
	puts("Mean time for single removal clash:");
	timespec_print(avg_time_rc, true);
	puts("");
}

struct timespec __map_bench_traffic()
{
	struct timespec timer;

	hashmap_t map = map_of_type(struct value, &hash_val);
	struct value *values = malloc(sizeof(struct value) * ITEM_CNT);
	for (size_t i = 0; i < ITEM_CNT; i++) {
		values[i] = (struct value){ .val = i };
	}

	size_t prev_cap = map.cap;
	size_t prev_i = 0;
	timer_start(&timer);
	for (size_t i = 0; i < ITEM_CNT; i++) {
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
	timer = timer_end(timer);

	assert(("unexpected map size", map.cnt != 0));

	free(values);
	map_free(&map);

	return timer;
}

void map_bench_traffic()
{
	struct timespec avg_time = __map_bench_traffic();

	for (size_t iter = 0; iter < BENCHMARK_ITERATIONS; iter++) {
		struct timespec time_taken = __map_bench_traffic();
		avg_time = timespec_avg(avg_time, time_taken);
	}

	puts("Mean time for bulk traffic:");
	timespec_print(avg_time, true);
	puts("");
}

struct timespec __map_bench_get()
{
	struct timespec timer;

	hashmap_t map = map_of_type(struct value, &hash_val);
	struct value *values = malloc(sizeof(struct value) * ITEM_CNT);
	for (size_t i = 0; i < ITEM_CNT; i++) {
		values[i] = (struct value){ .val = i };
		map_insert(&map, &(values[i]), &(values[i]));
	}

	size_t rand_val = rand() % ITEM_CNT;

	timer_start(&timer);
	map_get(&map, &(values[rand_val]));
	timer = timer_end(timer);

	free(values);
	map_free(&map);

	return timer;
}

void map_bench_get()
{
	struct timespec avg_time = __map_bench_traffic();

	for (size_t iter = 0; iter < BENCHMARK_ITERATIONS; iter++) {
		struct timespec time_taken = __map_bench_get();
		avg_time = timespec_avg(avg_time, time_taken);
	}

	puts("Mean time for rand get:");
	timespec_print(avg_time, true);
	puts("");
}

struct timespec __map_test_find()
{
	struct timespec timer;

	hashmap_t map = map_of_type(struct value, &hash_val);
	struct value *values = malloc(sizeof(struct value) * ITEM_CNT);
	for (size_t i = 0; i < ITEM_CNT; i++) {
		values[i] = (struct value){ .val = i };
		map_insert(&map, &(values[i]), &(values[i]));
	}

	size_t rand_val = rand() % ITEM_CNT;

	struct val_matcher matcher = (struct val_matcher) {
		.matcher = {
			.hash = hash_val(&values[rand_val]),
			.is_match = &match_on_num,
		},
		.match_val = values[rand_val].val,
	};

	timer_start(&timer);
	map_find(&map, (struct key_matcher *)&matcher);
	timer = timer_end(timer);

	map_free(&map);
	free(values);

	return timer;
}

void map_bench_find()
{
	struct timespec avg_time = __map_test_find();

	for (size_t i = 1; i < BENCHMARK_ITERATIONS; i++) {
		struct timespec time_taken = __map_test_find();

		avg_time = timespec_avg(avg_time, time_taken);
	}

	puts("Mean time for single find:");
	timespec_print(avg_time, true);
	puts("");
}
