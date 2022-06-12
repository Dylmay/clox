#ifndef __CLOX_OBJECT_FUNC_H__
#define __CLOX_OBJECT_FUNC_H__

#include "val/object.h"
#include "val_func.h"
#include "util/map/hash.h"

#define OBJECT_TYPE(obj) (VAL_AS_OBJ(obj)->type)
#define OBJECT_IS_STRING(obj) (object_is_type(obj, OBJ_STRING))
#define OBJECT_AS_STRING(obj) (((struct object_str *)VAL_AS_OBJ(obj)))
#define OBJECT_AS_CSTRING(obj) (((struct object_str *)VAL_AS_OBJ(obj))->chars)

static inline bool object_is_type(lox_val_t value, enum object_type type)
{
	return VAL_IS_OBJ(value) && VAL_AS_OBJ(value)->type == type;
}

void object_free(struct object *obj);
struct object_str *object_str_new(const char *chars, size_t len);
struct object_str *object_str_concat(const struct object_str *a,
				     const struct object_str *b);

bool object_equals(const struct object *a, const struct object *b);

void object_print(lox_val_t val);

hash_t obj_str_gen_hash(const void *key);

#endif // __CLOX_OBJECT_FUNC_H__