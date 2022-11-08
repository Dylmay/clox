#include "object_func.h"

#include "util/mem/mem.h"
#include "util/map/hash_util.h"
#include "chunk/func/chunk_func.h"
#include "util/map/set.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define ALLOCATE_OBJECT(type, type_id)                                         \
	((type *)__allocate_object(sizeof(type), type_id))

#define ALLOCATE_OBJECT_STR(str_sz)                                            \
	((struct object_str *)__allocate_object(                               \
		sizeof(struct object_str) + (sizeof(char) * (str_sz + 1)),     \
		OBJ_STRING))

struct object_str_matcher {
	struct key_matcher m;
	const char *chars;
	size_t len;
};

static bool __match(const void *a, struct key_matcher *m);
static struct object_str_matcher __create_matcher(const char *chars,
						  size_t len);
static struct object_str *__create_object_str(const char *, size_t);
static struct object *__allocate_object(size_t, enum object_type);
static struct object_str *__intern_string(const char *chars, size_t len);
static void __print_function(struct object_fn *fn);

static hashset_t interner = hashset_new((hash_fn)&obj_str_gen_hash);

struct object_str *object_str_new(const char *chars, size_t len)
{
	return __intern_string(chars, len);
}

struct object_fn *object_fn_new()
{
	struct object_fn *fn = ALLOCATE_OBJECT(struct object_fn, OBJ_FN);

	fn->arity = 0;
	fn->name = NULL;
	fn->chunk = chunk_new();

	return fn;
}

struct object_str *object_str_concat(const struct object_str *a,
				     const struct object_str *b)
{
	size_t concat_len = a->len + b->len;
	char *concat_str = reallocate(NULL, 0, concat_len);

	memcpy(concat_str, a->chars, a->len);
	memcpy(concat_str + a->len, b->chars, b->len);
	concat_str[concat_len] = '\0';

	struct object_str *concat = __intern_string(concat_str, concat_len);
	reallocate(concat_str, concat_len, 0);

	return concat;
}

struct object_native_fn *object_native_fn_new(native_fn native_fn)
{
	struct object_native_fn *native =
		ALLOCATE_OBJECT(struct object_native_fn, OBJ_NATIVE);
	native->fn = native_fn;

	return native;
}

struct object_closure *object_closure_new(struct object_fn *fn)
{
	struct object_closure *closure =
		ALLOCATE_OBJECT(struct object_closure, OBJ_CLOSURE);

	closure->fn = fn;

	return closure;
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

	case OBJ_FN: {
		struct object_fn *fn = (struct object_fn *)obj;
		chunk_free(&fn->chunk);
		FREE(struct object_fn, fn);
	} break;

	case OBJ_NATIVE:
		FREE(struct object_native_fn, obj);
		break;

	case OBJ_CLOSURE:
		FREE(struct object_closure, obj);
		break;

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

	case OBJ_FN:
		__print_function(OBJECT_AS_FN(val));
		break;

	case OBJ_NATIVE:
		printf("<native fn>");
		break;

	case OBJ_CLOSURE:
		__print_function(OBJECT_AS_CLOSURE(val)->fn);
		break;

	default:
		assert(("Unknown object type", 0));
		break;
	}
}

static void __print_function(struct object_fn *fn)
{
	if (fn->name == NULL) {
		printf("<script>");
	} else {
		printf("<fn %s>", fn->name->chars);
	}
}

lox_val_t object_to_string(lox_val_t val)
{
	switch (OBJECT_TYPE(val)) {
	case OBJ_STRING:
		return val;

	case OBJ_FN: {
		struct object_fn *fn = OBJECT_AS_FN(val);

		if (fn->name == NULL) {
			return VAL_CREATE_OBJ(object_str_new(
				"<script>", sizeof("<script>") - 1));
		} else {
			size_t len = (sizeof("<fn >") - 1) + fn->name->len;
			char *concat_str = reallocate(NULL, 0, len);
			snprintf(concat_str, len, "<fn %s>", fn->name->chars);
			struct object_str *interned_str =
				object_str_new(concat_str, len);
			reallocate(concat_str, len, 0);

			return VAL_CREATE_OBJ(interned_str);
		}
	} break;

	case OBJ_NATIVE:
		return VAL_CREATE_OBJ(object_str_new(
			"<native fn>", sizeof("<native fn>") - 1));
		break;

	default:
		assert(("Unknown object type", 0));
		return VAL_CREATE_OBJ(
			object_str_new("<unknown>", sizeof("<unknown>") - 1));
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

hash_t obj_str_gen_hash(const struct object_str *str)
{
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

static bool __match(const void *a, struct key_matcher *m)
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
		.m = { .is_match = &__match,
		       .hash = c_str_gen_hash(chars, len) },
		.len = len,
		.chars = chars,
	};
}

static struct object_str *__intern_string(const char *chars, size_t len)
{
	struct object_str_matcher matcher = __create_matcher(chars, len);
	struct object_str *interned =
		hashset_find(&interner, (struct key_matcher *)&matcher);

	if (!interned) {
		interned = __create_object_str(chars, len);
		hashset_insert(&interner, interned);
	}

	return interned;
}