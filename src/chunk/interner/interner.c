#include "interner.h"

#include <string.h>

#include "val/func/object_func.h"
#include "util/map/hash_util.h"

struct object_str_matcher {
	key_matcher_t m;
	const char *chars;
	size_t len;
};

bool match(const void *a, key_matcher_t *m);
static struct object_str_matcher __create_matcher(const char *chars,
						  size_t len);

interner_t intern_new()
{
	return set_new(&obj_str_gen_hash);
}

void intern_free(interner_t *interner)
{
	set_for_each_t for_each = (set_for_each_t){
		.func = (void (*)(void *, set_for_each_t *)) & object_free,
	};

	set_for_each(interner, &for_each);
	set_free(interner);
}

struct object_str *intern_string(interner_t *interner, const char *chars,
				 size_t len)
{
	struct object_str_matcher matcher = __create_matcher(chars, len);
	struct object_str *interned =
		set_find(interner, (key_matcher_t *)&matcher);

	if (!interned) {
		interned = object_str_new(chars, len);
		set_insert(interner, interned);
	}

	return interned;
}

struct object_str *intern_get_str(const interner_t *interner,
				  const struct object_str *str)
{
	struct object_str_matcher matcher =
		__create_matcher(str->chars, str->len);

	return set_find(interner, (key_matcher_t *)&matcher);
}

bool intern_insert(interner_t *interner, struct object_str *str)
{
	return set_insert(interner, str);
}

bool match(const void *a, key_matcher_t *m)
{
	const struct object_str *str = (const struct object_str *)a;
	const struct object_str_matcher *matcher =
		(const struct object_str_matcher *)m;

	return str->len == matcher->len &&
	       memcmp(str->chars, matcher->chars, matcher->len) == 0;
}

static struct object_str_matcher __create_matcher(const char *chars, size_t len)
{
	return (struct object_str_matcher){
		.m = { .is_match = &match, .hash = c_str_gen_hash(chars, len) },
		.len = len,
		.chars = chars,
	};
}