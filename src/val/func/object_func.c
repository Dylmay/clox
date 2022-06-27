#include "object_func.h"

#include "util/mem/mem.h"
#include "util/map/hash_util.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define ALLOCATE_OBJECT(type, type_id)                                         \
	((type *)__allocate_object(sizeof(type), type_id))

#define ALLOCATE_OBJECT_STR(str_sz)                                            \
	((struct object_str *)__allocate_object(                               \
		sizeof(struct object_str) + (sizeof(char) * (str_sz + 1)),     \
		OBJ_STRING))

static struct object_str *__create_object_str(const char *, size_t);
static struct object *__allocate_object(size_t, enum object_type);

struct object_str *object_str_new(const char *chars, size_t len)
{
	return __create_object_str(chars, len);
}

struct object_str *object_str_concat(const struct object_str *a,
				     const struct object_str *b)
{
	size_t new_len = a->len + b->len;
	struct object_str *concat = ALLOCATE_OBJECT_STR(new_len);

	memcpy(concat->chars, a->chars, a->len);
	memcpy(concat->chars + a->len, b->chars, b->len);
	concat->chars[new_len] = '\0';
	concat->len = new_len;

	return concat;
}

void object_free(struct object *obj)
{
	switch (obj->type) {
	case OBJ_STRING: {
		struct object_str *str = (struct object_str *)obj;
		reallocate(str,
			   sizeof(struct object_str) +
				   (sizeof(char) * str->len),
			   0);
	} break;

	default:
		break;
	}
}

void object_print(lox_val_t val)
{
	switch (OBJECT_TYPE(val)) {
	case OBJ_STRING:
		printf("%s", OBJECT_AS_CSTRING(val));
		break;

	default:
		assert(("Unknown object type", 0));
		break;
	}
}

bool object_equals(const struct object *a, const struct object *b)
{
	if (a->type != b->type) {
		return false;
	}

	switch (a->type) {
	case OBJ_STRING:
		return a == b;
	default:
		assert(("Unknown object comparison type", 0));
		return false;
	}
}

hash_t obj_str_gen_hash(const void *key)
{
	const struct object_str *str = (const struct object_str *)key;

	return c_str_gen_hash(str->chars, str->len);
}

static struct object_str *__create_object_str(const char *chars, size_t str_sz)
{
	struct object_str *string = ALLOCATE_OBJECT_STR(str_sz);

	string->len = str_sz;
	memcpy(string->chars, chars, str_sz);
	string->chars[str_sz] = '\0';

	return string;
}

static struct object *__allocate_object(size_t obj_sz,
					enum object_type obj_type)
{
	struct object *obj = (struct object *)reallocate(NULL, 0, obj_sz);
	obj->type = obj_type;

	return obj;
}
