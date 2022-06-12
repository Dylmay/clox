#include "interner.h"

#include <string.h>

#include "val/func/object_func.h"
#include "util/hash_util.h"

struct object_str_matcher {
	matcher_t m;
	const char *chars;
	size_t len;
};

bool match(const void *a, matcher_t *m);
static struct object_str_matcher __create_matcher(const char *chars,
						  size_t len);

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
	struct object_str_matcher matcher = __create_matcher(chars, len);
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

static struct object_str_matcher __create_matcher(const char *chars, size_t len)
{
	return (struct object_str_matcher){
		.m = { .is_match = &match, .hash = str_gen_hash(chars, len) },
		.len = len,
		.chars = chars,
	};
}