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

static hashmap_t __lookup_scope_new();
static struct name_matcher __create_matcher(const char *name, size_t len);
static lookup_var_t *__lookup_find_name_ptr(lookup_t *lookup, const char *name,
					    size_t len);
static lookup_var_t *__lookup_scope_find_name_ptr(lookup_t *lookup,
						  const char *name, size_t len);
static size_t __lookup_inc_global_cnt(lookup_t *lookup);
static size_t __lookup_inc_local_cnt(lookup_t *lookup);

void lookup_entry_free(struct map_entry entry, struct map_for_each_entry *data)
{
	string_free(entry.key);
}

void scope_free(const void *m)
{
	struct map_for_each_entry lookup_free = (struct map_for_each_entry){
		.func = &lookup_entry_free,
	};

	hashmap_t *map = (hashmap_t *)m;
	map_entries_for_each(map, (struct map_for_each_entry *)&lookup_free);
	map_free(map);
}

bool match_key(const void *key, struct key_matcher *m)
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
		.glbl_idx = 0,
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

lookup_var_t lookup_declare(lookup_t *lookup, size_t scope_depth,
			    const char *chars, size_t len, bool mutable)
{
	assert(("scope starts at 1", scope_depth));

	struct string *name = string_new(chars, len);

	bool is_glbl = scope_depth == LOOKUP_GLOBAL_DEPTH;

	var_flags_t flags =
		LOOKUP_VAR_DECLARED |
		(is_glbl ? LOOKUP_VAR_GLOBAL : LOOKUP_VAR_LOCAL) |
		(mutable ? LOOKUP_VAR_MUTABLE : LOOKUP_VAR_IMMUTABLE);

	lookup_var_t var = (lookup_var_t){

		.idx = is_glbl ? __lookup_inc_global_cnt(lookup) :
				 __lookup_inc_local_cnt(lookup),
		.var_flags = flags,
	};

	map_insert(lookup_scope_at_depth(lookup, scope_depth), name, &var);

	return var;
}

lookup_var_t lookup_define(lookup_t *lookup, const char *chars, size_t len,
			   bool mutable)
{
	struct string *name = string_new(chars, len);

	lookup_var_t *reserved =
		__lookup_scope_find_name_ptr(lookup, chars, len);

	if (reserved) {
		if (lookup_var_is_defined(*reserved)) {
			return (lookup_var_t){
				.idx = 0,
				.var_flags = LOOKUP_VAR_INVALID,
			};
		}

		reserved->var_flags |= LOOKUP_VAR_DEFINED;
		return *reserved;
	}
	bool is_glbl = lookup_cur_depth(lookup) == LOOKUP_GLOBAL_DEPTH;

	var_flags_t flags =
		LOOKUP_VAR_DECLARED | LOOKUP_VAR_DEFINED |
		(is_glbl ? LOOKUP_VAR_GLOBAL : LOOKUP_VAR_LOCAL) |
		(mutable ? LOOKUP_VAR_MUTABLE : LOOKUP_VAR_IMMUTABLE);

	lookup_var_t var = (lookup_var_t){

		.idx = is_glbl ? __lookup_inc_global_cnt(lookup) :
				 __lookup_inc_local_cnt(lookup),
		.var_flags = flags,
	};

	map_insert(lookup_cur_scope(lookup), name, &var);

	return var;
}

bool lookup_scope_has_name(lookup_t *lookup, const char *name, size_t len)
{
	struct name_matcher matcher = __create_matcher(name, len);
	lookup_var_t *var = map_find_by_key(lookup_cur_scope(lookup),
					    (struct key_matcher *)&matcher)
				    .value;

	return var && lookup_var_is_defined(*var);
}

lookup_var_t lookup_find_name(lookup_t *lookup, const char *name, size_t len)
{
	lookup_var_t *var = __lookup_find_name_ptr(lookup, name, len);

	return var ? *var :
		     (lookup_var_t){ .var_flags = LOOKUP_VAR_NOT_DECLARED };
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

hashmap_t *lookup_scope_at_depth(lookup_t *lookup, size_t idx)
{
	assert(("Indexing begins at 1", idx));

	return (hashmap_t *)list_get(&lookup->scopes, idx - 1);
}

static hashmap_t __lookup_scope_new()
{
	return map_of_type(lookup_var_t, (hash_fn)&string_gen_hash);
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

static lookup_var_t *__lookup_scope_find_name_ptr(lookup_t *lookup,
						  const char *name, size_t len)
{
	struct name_matcher matcher = __create_matcher(name, len);
	lookup_var_t *found = map_find_by_key(lookup_cur_scope(lookup),
					      (struct key_matcher *)&matcher)
				      .value;

	return found;
}

static lookup_var_t *__lookup_find_name_ptr(lookup_t *lookup, const char *name,
					    size_t len)
{
	struct name_matcher matcher = __create_matcher(name, len);

	for (size_t depth = lookup_cur_depth(lookup); depth >= 1; depth--) {
		const hashmap_t *scope = lookup_scope_at_depth(lookup, depth);
		lookup_var_t *found =
			map_find_by_key(scope, (struct key_matcher *)&matcher)
				.value;

		if (found) {
			return found;
		}
	}

	return NULL;
}

static size_t __lookup_inc_global_cnt(lookup_t *lookup)
{
	return lookup->glbl_idx++;
}

static size_t __lookup_inc_local_cnt(lookup_t *lookup)
{
	return lookup->cur_idx++;
}
