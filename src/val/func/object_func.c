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
	((lox_str_t *)__allocate_object(sizeof(lox_str_t) +                    \
						(sizeof(char) * (str_sz + 1)), \
					OBJ_STRING))

#define UNKNOWN_STR "<unknown>"
#define SCRIPT_STR "<script>"
#define CLASS_STR "<class %s>"
#define INSTANCE_STR "<instance %s>"

struct lox_str_t_matcher {
	struct key_matcher m;
	const char *chars;
	size_t len;
};

static bool __match(const void *a, struct key_matcher *m);
static struct lox_str_t_matcher __create_matcher(const char *chars, size_t len);
static lox_str_t *__create_object_str(const char *, size_t);
static lox_obj_t *__allocate_object(size_t, enum object_type);
static lox_str_t *__intern_string(const char *chars, size_t len);
static void __print_function(lox_fn_t *fn);

static hashset_t interner = hashset_new((hash_fn)&obj_str_gen_hash);

lox_str_t *object_str_new(const char *chars, size_t len)
{
	return __intern_string(chars, len);
}

lox_fn_t *object_fn_new(lox_str_t *name)
{
	lox_fn_t *fn = ALLOCATE_OBJECT(lox_fn_t, OBJ_FN);

	fn->arity = 0;
	fn->upval_cnt = 0;
	fn->name = name;
	fn->chunk = chunk_new();

	return fn;
}

lox_str_t *object_str_concat(const lox_str_t *a, const lox_str_t *b)
{
	size_t concat_len = a->len + b->len;
	char *concat_str = reallocate(NULL, 0, concat_len);

	memcpy(concat_str, a->chars, a->len);
	memcpy(concat_str + a->len, b->chars, b->len);
	concat_str[concat_len] = '\0';

	lox_str_t *concat = __intern_string(concat_str, concat_len);
	reallocate(concat_str, concat_len, 0);

	return concat;
}

lox_native_t *object_native_fn_new(const native_import_t native_import)
{
	lox_native_t *native = ALLOCATE_OBJECT(lox_native_t, OBJ_NATIVE);
	native->import = native_import;

	return native;
}

lox_closure_t *object_closure_new(lox_fn_t *fn)
{
	lox_closure_t *closure = ALLOCATE_OBJECT(lox_closure_t, OBJ_CLOSURE);

	closure->fn = fn;
	closure->upvalues = list_of_type(lox_upval_t);
	list_set_cap(&closure->upvalues, fn->upval_cnt);

	return closure;
}

lox_class_t *object_class_new(lox_str_t *cls_name)
{
	lox_class_t *cls = ALLOCATE_OBJECT(lox_class_t, OBJ_CLASS);

	cls->name = cls_name;

	cls->field_lookup.table = lookup_new();
	cls->field_lookup.idx = 0;

	cls->static_lookup.table = lookup_new();
	cls->static_lookup.idx = 0;
	cls->statics = list_of_type(lox_val_t);

	return cls;
}

lox_instance_t *object_instance_new(lox_class_t *cls)
{
	lox_instance_t *instance =
		ALLOCATE_OBJECT(lox_instance_t, OBJ_INSTANCE);

	instance->cls = cls;
	instance->fields = list_of_type(lox_val_t);
	size_t field_size = map_size(&cls->field_lookup.table.table);
	list_set_cnt(&instance->fields, field_size);

	return instance;
}

lox_upval_t *object_upval_new(lox_val_t *slot)
{
	lox_upval_t *upval = ALLOCATE_OBJECT(lox_upval_t, OBJ_UPVALUE);

	upval->location = slot;
	upval->closed = VAL_CREATE_NIL;
	upval->next = NULL;

	return upval;
}

void object_free(struct object *obj)
{
	switch (obj->type) {
	case OBJ_STRING: {
		lox_str_t *str = (lox_str_t *)obj;
		reallocate(str, sizeof(lox_str_t) + (sizeof(char) * str->len),
			   0);
	} break;

	case OBJ_FN: {
		lox_fn_t *fn = (lox_fn_t *)obj;
		chunk_free(&fn->chunk);
		FREE(lox_fn_t, fn);
	} break;

	case OBJ_NATIVE:
		FREE(lox_native_t, obj);
		break;

	case OBJ_CLOSURE: {
		lox_closure_t *closure = (lox_closure_t *)obj;
		list_free(&closure->upvalues);
		FREE(lox_closure_t, obj);
	} break;

	case OBJ_UPVALUE:
		FREE(lox_upval_t, obj);
		break;

	case OBJ_INSTANCE: {
		lox_instance_t *instance = (lox_instance_t *)obj;

		list_free(&instance->fields);
		FREE(lox_instance_t, obj);
		break;
	}

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

	case OBJ_NATIVE: {
		lox_native_t *fn = OBJECT_AS_NATIVE(val);
		printf("<native_fn %s>", fn->import.fn_name);
	} break;

	case OBJ_CLOSURE:
		__print_function(OBJECT_AS_CLOSURE(val)->fn);
		break;

	case OBJ_CLASS:
		printf(CLASS_STR, OBJECT_AS_CLASS(val)->name->chars);
		break;

	case OBJ_INSTANCE:
		printf(INSTANCE_STR, OBJECT_AS_INSTANCE(val)->cls->name->chars);
		break;

	case OBJ_UPVALUE:
		val_print(*OBJECT_AS_UPVALUE(val)->location);
		break;

	default:
		assert(("Unknown object type", 0));
		break;
	}
}

static void __print_function(lox_fn_t *fn)
{
	if (fn->name == NULL) {
		printf(SCRIPT_STR);
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
		lox_fn_t *fn = OBJECT_AS_FN(val);
		lox_str_t *string;

		if (fn->name == NULL) {
			string = object_str_new(SCRIPT_STR,
						sizeof(SCRIPT_STR) - 1);
		} else {
			size_t len = (sizeof("<fn >") - 1) + fn->name->len;
			char *concat_str = reallocate(NULL, 0, len);
			snprintf(concat_str, len, "<fn %s>", fn->name->chars);
			string = object_str_new(concat_str, len);

			reallocate(concat_str, len, 0);
		}

		return VAL_CREATE_OBJ(string);
	}

	case OBJ_CLASS:
		return VAL_CREATE_OBJ(OBJECT_AS_CLASS(val)->name);

	case OBJ_NATIVE: {
		{
			size_t len = (sizeof("<native_fn >") - 1) +
				     OBJECT_AS_NATIVE(val)->import.name_sz;

			char *concat_str = reallocate(NULL, 0, len);
			snprintf(concat_str, len, "<native_fn %s>",
				 OBJECT_AS_NATIVE(val)->import.fn_name);

			lox_str_t *string = object_str_new(concat_str, len);

			reallocate(concat_str, len, 0);

			return VAL_CREATE_OBJ(string);
		}
	} break;

	case OBJ_UPVALUE:
		return object_to_string(*OBJECT_AS_UPVALUE(val)->location);

	case OBJ_INSTANCE:
		return VAL_CREATE_OBJ(OBJECT_AS_INSTANCE(val)->cls->name);

	default:
		assert(("Unknown object type", 0));
		return VAL_CREATE_OBJ(
			object_str_new(UNKNOWN_STR, sizeof(UNKNOWN_STR) - 1));
	}
}

bool object_equals(const struct object *a, const struct object *b)
{
	if (a->type != b->type) {
		return false;
	}

	switch (a->type) {
	default:
		assert(("Unknown object comparison type", 0));
		return false;
	case OBJ_STRING:
	case OBJ_FN:
	case OBJ_CLOSURE:
	case OBJ_NATIVE:
		return a == b;
	}
}

hash_t obj_str_gen_hash(const lox_str_t *str)
{
	return c_str_gen_hash(str->chars, str->len);
}

static lox_str_t *__create_object_str(const char *chars, size_t str_sz)
{
	lox_str_t *string = ALLOCATE_OBJECT_STR(str_sz);

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
	const lox_str_t *str = (const lox_str_t *)a;
	const struct lox_str_t_matcher *matcher =
		(const struct lox_str_t_matcher *)m;

	return str->len == matcher->len &&
	       memcmp(str->chars, matcher->chars, matcher->len) == 0;
}

static struct lox_str_t_matcher __create_matcher(const char *chars, size_t len)
{
	return (struct lox_str_t_matcher){
		.m = { .is_match = &__match,
		       .hash = c_str_gen_hash(chars, len) },
		.len = len,
		.chars = chars,
	};
}

static lox_str_t *__intern_string(const char *chars, size_t len)
{
	struct lox_str_t_matcher matcher = __create_matcher(chars, len);
	lox_str_t *interned =
		hashset_find(&interner, (struct key_matcher *)&matcher);

	if (!interned) {
		interned = __create_object_str(chars, len);
		hashset_insert(&interner, interned);
	}

	return interned;
}