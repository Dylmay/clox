#include "lookup.h"

#include <assert.h>
#include <string.h>

#include "val/func/object_func.h"
#include "util/string/string_util.h"

struct name_matcher {
	const key_matcher_t m;
	const char *str;
	size_t strlen;
};

static hashmap_t __lookup_scope_new();
static struct name_matcher __create_matcher(const char *name, size_t len);

void lookup_entry_free(map_entry_t entry, for_each_entry_t *data)
{
	string_free(entry.key);
}

void scope_free(const void *m)
{
	for_each_entry_t lookup_free = (for_each_entry_t){
		.func = &lookup_entry_free,
	};

	hashmap_t *map = (hashmap_t *)m;
	map_entries_for_each(map, (for_each_entry_t *)&lookup_free);
	map_free(map);
}

bool match_key(const void *key, key_matcher_t *m)
{
	struct string *str = (struct string *)key;
	struct name_matcher *matcher = (struct name_matcher *)m;

	return string_get_len(str) == matcher->strlen &&
	       strncmp(string_get_cstring(str), matcher->str,
		       matcher->strlen) == 0;
}

lookup_t lookup_new()
{
	list_t scopes = list_of_type(hashmap_t);
	list_set_cap(&scopes, 1);

	lookup_t lookup = (lookup_t){
		.scopes = list_of_type(hashmap_t),
		.cur_idx = 0,
	};
	lookup_begin_scope(&lookup);

	return lookup;
}

void lookup_free(lookup_t *lookup)
{
	list_for_each (&lookup->scopes, (for_each_fn)&scope_free)
		;
	list_free(&lookup->scopes);
}

lookup_var_t lookup_scope_define(lookup_t *lookup, const char *name, size_t len,
				 var_flags_t flags)
{
	lookup_var_t var = (lookup_var_t){
		.idx = lookup->cur_idx,
		.var_flags = flags,
	};

	lookup->cur_idx++;

	struct string *str = string_new(name, len);

	map_insert(lookup_cur_scope(lookup), str, &var);

	return var;
}

bool lookup_scope_has_name(lookup_t *lookup, const char *name, size_t len)
{
	struct name_matcher matcher = __create_matcher(name, len);

	return map_find_by_key(lookup_cur_scope(lookup),
			       (key_matcher_t *)&matcher)
		.value;
}

lookup_var_t lookup_find_name(lookup_t *lookup, const char *name, size_t len)
{
	struct name_matcher matcher = __create_matcher(name, len);

	for (size_t depth = lookup_cur_depth(lookup); depth >= 1; depth--) {
		const hashmap_t *scope = lookup_scope_at_depth(lookup, depth);
		const lookup_var_t *found =
			map_find_by_key(scope, (key_matcher_t *)&matcher).value;

		if (found) {
			return *found;
		}
	}

	return (lookup_var_t){ .var_flags = LOOKUP_VAR_INVALID };
}

void lookup_begin_scope(lookup_t *lookup)
{
	hashmap_t scope = __lookup_scope_new();
	list_push(&lookup->scopes, &scope);
}

void lookup_end_scope(lookup_t *lookup)
{
	assert(("lookup depth must be over 1", lookup_cur_depth(lookup) > 1));
	hashmap_t *scope = (hashmap_t *)list_pop(&lookup->scopes);
	lookup->cur_idx -= map_size(scope);

	map_free(scope);
}

size_t lookup_cur_depth(const lookup_t *lookup)
{
	return list_size(&lookup->scopes);
}

hashmap_t *lookup_cur_scope(lookup_t *lookup)
{
	return (hashmap_t *)list_peek(&lookup->scopes);
}

static hashmap_t __lookup_scope_new()
{
	return map_of_type(lookup_var_t, (hash_fn)&string_gen_hash);
}

hashmap_t *lookup_scope_at_depth(lookup_t *lookup, size_t idx)
{
	assert(("Indexing begins at 1", idx));

	return (hashmap_t *)list_get(&lookup->scopes, idx - 1);
}

static struct name_matcher __create_matcher(const char *name, size_t len)
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