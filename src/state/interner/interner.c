#include "interner.h"

#include <string.h>

#include "val/func/object_func.h"
#include "util/map/hash_util.h"

struct object_str_matcher {
	struct key_matcher m;
	const char *chars;
	size_t len;
};

bool _match(const void *a, struct key_matcher *m);
static struct object_str_matcher __create_matcher(const char *chars,
						  size_t len);

interner_t intern_new()
{
	return hashset_new((hash_fn)&obj_str_gen_hash);
}

void intern_free(interner_t *interner)
{
	struct hashset_for_each for_each = (struct hashset_for_each){
		.func = (void (*)(void *, struct hashset_for_each *)) &
			object_free,
	};

	hashset_for_each(interner, &for_each);
	hashset_free(interner);
}

struct object_str *intern_string(interner_t *interner, const char *chars,
				 size_t len)
{
	struct object_str_matcher matcher = __create_matcher(chars, len);
	struct object_str *interned =
		hashset_find(interner, (struct key_matcher *)&matcher);

	if (!interned) {
		interned = object_str_new(chars, len);
		hashset_insert(interner, interned);
	}

	return interned;
}

struct object_str *intern_get_str(const interner_t *interner, const char *chars,
				  size_t len)
{
	struct object_str_matcher matcher = __create_matcher(chars, len);

	return hashset_find(interner, (struct key_matcher *)&matcher);
}

bool _match(const void *a, struct key_matcher *m)
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
		.m = { .is_match = &_match,
		       .hash = c_str_gen_hash(chars, len) },
		.len = len,
		.chars = chars,
	};
}