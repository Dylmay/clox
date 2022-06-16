#include "lookup.h"
#include "val/func/object_func.h"

struct name_matcher {
	key_matcher_t m;
	lox_str_t *expected;
};

struct val_matcher {
	val_matcher_t m;
	uint32_t val;
};

bool match_key(const void *key, key_matcher_t *m)
{
	lox_str_t *matched = (const lox_str_t *)key;
	struct name_matcher *matcher = (struct name_matcher *)m;

	return object_equals((lox_obj_t *)matcher->expected,
			     (lox_obj_t *)matched);
}

bool match_val(const void *val, val_matcher_t *m)
{
	uint32_t matched = *((const uint32_t *)val);
	const struct val_matcher *matcher = (const struct val_matcher *)m;

	return matched == matcher->val;
}

lookup_t lookup_new()
{
	return map_of_type(int, (hash_fn)&obj_str_gen_hash);
}

uint32_t lookup_by_name(lookup_t *lookup, const lox_str_t *name)
{
	uint32_t idx;
	const uint32_t *found = map_get(lookup, name);

	if (found) {
		idx = *found;
	} else {
		idx = lookup_size(lookup);
		map_insert(lookup, name, &idx);
	}

	return idx;
}

lox_str_t *lookup_find(const lookup_t *lookup, uint32_t idx)
{
	struct val_matcher matcher = {
		.m = {
			.is_match = &match_val,
		},
		.val = idx,
	};

	return map_find_key(lookup, (val_matcher_t *)&matcher);
}

bool lookup_has_name(const lookup_t *lookup, const lox_str_t *name)
{
	struct name_matcher matcher = {
		.m = {
			.hash = obj_str_gen_hash(name),
			.is_match = &match_key,
		},
		.expected = name,
	};

	return map_find(lookup, (key_matcher_t *)&matcher) != NULL;
}

lookup_t *lookup_new_scope(lookup_t *lookup)
{
}

lookup_t *lookup_end_scope(lookup_t *lookup)
{
}