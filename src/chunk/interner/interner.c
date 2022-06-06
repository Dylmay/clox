#include "interner.h"

#include <string.h>

#include "val/func/object_func.h"

struct object_str_matcher {
	matcher_t m;
	const char *chars;
	size_t len;
};

struct object_str_matcher create_matcher(const char *chars, size_t len);
bool match(const void *a, matcher_t *m);
hash_t str_gen_hash(const char *chars, size_t str_sz);
hash_t obj_str_gen_hash(const void *key);

interner_t intern_new()
{
	return set_new(&obj_str_gen_hash);
}

void intern_free(interner_t *interner)
{
	set_for_each(interner, &object_free);
	set_free(interner);
}

struct object_str *intern_string(interner_t *interner, const char *chars,
				 size_t len)
{
	struct object_str_matcher matcher = create_matcher(chars, len);
	struct object_str *interned = set_find(interner, (matcher_t *)&matcher);

	if (!interned) {
		interned = object_str_new(chars, len);
		set_insert(interner, interned);
	}

	return interned;
}

bool match(const void *a, matcher_t *m)
{
	const struct object_str *str = (struct object_str *)a;
	const struct object_str_matcher *matcher =
		(struct object_str_matcher *)m;

	return str->len == matcher->len &&
	       memcmp(str->chars, matcher->chars, matcher->len) == 0;
}

struct object_str_matcher create_matcher(const char *chars, size_t len)
{
	return (struct object_str_matcher){
		.m = { .is_match = &match, .hash = str_gen_hash(chars, len) },
		.len = len,
		.chars = chars,
	};
}

hash_t str_gen_hash(const char *chars, size_t str_sz)
{
	uint32_t hash = 216613626UL;

	for (size_t i = 0; i < str_sz; i++) {
		hash ^= (uint8_t)chars[i];
		hash *= 16777619;
	}

	return hash;
}

hash_t obj_str_gen_hash(const void *key)
{
	const struct object_str *str = (const struct object_str *)key;

	return str_gen_hash(str->chars, str->len);
}
