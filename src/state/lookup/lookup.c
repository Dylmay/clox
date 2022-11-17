#include "lookup.h"

#include <assert.h>
#include <string.h>

#include "val/func/object_func.h"
#include "util/string/string_util.h"

struct name_matcher {
	const struct key_matcher m;
	const char *str;
	size_t strlen;
};

struct idx_matcher {
	const struct val_matcher v;
	uint32_t idx;
};

static struct name_matcher __create_name_matcher(const char *name, size_t len);
static struct idx_matcher __create_idx_matcher(uint32_t idx);
static lookup_var_t *__lookup_find_name_ptr(const lookup_t *lookup,
					    const char *name, size_t len);
static uint32_t __lookup_inc_cnt(lookup_t *lookup);

bool match_key(const void *key, struct key_matcher *m)
{
	struct string *str = (struct string *)key;
	const struct name_matcher *matcher = (const struct name_matcher *)m;

	return string_get_len(str) == matcher->strlen &&
	       strncmp(string_get_cstring(str), matcher->str,
		       matcher->strlen) == 0;
}

bool match_value(const void *val, struct val_matcher *m)
{
	const struct idx_matcher *matcher = (const struct idx_matcher *)m;
	lookup_var_t *var = (lookup_var_t *)val;

	return var->idx == matcher->idx;
}

lookup_t lookup_new(uint32_t idx_start)
{
	lookup_t lookup = (lookup_t){
		.table = map_of_type(lookup_var_t, (hash_fn)&string_gen_hash),
		.idx = idx_start,
	};

	return lookup;
}

void lookup_free(lookup_t *lookup)
{
	map_free(&lookup->table);
	*lookup = (lookup_t){ 0 };
}

lookup_var_t lookup_declare(lookup_t *lookup, const char *chars, size_t len,
			    var_flags_t flags)
{
	struct string *name = string_new(chars, len);

	flags |= LOOKUP_VAR_DECLARED;

	lookup_var_t var = (lookup_var_t){

		.idx = __lookup_inc_cnt(lookup),
		.var_flags = flags,
	};

	map_insert(&lookup->table, name, &var);

	return var;
}

bool lookup_remove(lookup_t *lookup, uint32_t idx)
{
	struct idx_matcher matcher = __create_idx_matcher(idx);

	struct map_entry entry = map_find_by_value(
		&lookup->table, (struct val_matcher *)&matcher);

	if (!entry.key) {
		return false;
	}

	return map_remove(&lookup->table, entry.key);
}

lookup_var_t lookup_define(lookup_t *lookup, const char *chars, size_t len,
			   var_flags_t flags)
{
	struct string *name = string_new(chars, len);

	lookup_var_t *reserved = __lookup_find_name_ptr(lookup, chars, len);

	if (reserved) {
		if (lookup_var_is_defined(*reserved)) {
			return LOOKUP_VAR_TYPE_INVALID;
		}

		reserved->var_flags |= LOOKUP_VAR_DEFINED;
		return *reserved;
	}

	flags |= LOOKUP_VAR_DECLARED;
	flags |= LOOKUP_VAR_DEFINED;

	lookup_var_t var = (lookup_var_t){

		.idx = __lookup_inc_cnt(lookup),
		.var_flags = flags,
	};

	map_insert(&lookup->table, name, &var);

	return var;
}

bool lookup_has_name(const lookup_t *lookup, const char *name, size_t len)
{
	struct name_matcher matcher = __create_name_matcher(name, len);
	const lookup_var_t *var =
		map_find_by_key(&lookup->table, (struct key_matcher *)&matcher)
			.value;

	return var && lookup_var_is_defined(*var);
}

lookup_var_t lookup_find_name(const lookup_t *lookup, const char *name,
			      size_t len)
{
	const lookup_var_t *var = __lookup_find_name_ptr(lookup, name, len);

	return var ? *var :
		     (lookup_var_t){ .var_flags = LOOKUP_VAR_NOT_DECLARED };
}

static struct name_matcher __create_name_matcher(const char *name, size_t len)
{
	return (struct name_matcher){
		.m = {
			.hash = c_str_gen_hash(name, len),
			.is_match = &match_key,
		},
		.str = name,
		.strlen = len,
	};
}

static struct idx_matcher __create_idx_matcher(uint32_t idx)
{
	return (struct idx_matcher){
		.v = { .is_match = &match_value },
		.idx = idx,
	};
}

static lookup_var_t *__lookup_find_name_ptr(const lookup_t *lookup,
					    const char *name, size_t len)
{
	struct name_matcher matcher = __create_name_matcher(name, len);
	lookup_var_t *found =
		map_find_by_key(&lookup->table, (struct key_matcher *)&matcher)
			.value;

	return found;
}

static uint32_t __lookup_inc_cnt(lookup_t *lookup)
{
	assert(("Lookup count will overflow", lookup->idx != UINT32_MAX));
	return lookup->idx++;
}
