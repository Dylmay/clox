#ifndef __CLOX_OBJECT_FUNC_H__
#define __CLOX_OBJECT_FUNC_H__

#include "val/object.h"
#include "val_func.h"
#include "util/map/hash.h"

#define OBJECT_TYPE(obj) (VAL_AS_OBJ(obj)->type)
#define OBJECT_IS_STRING(obj) (object_is_type(obj, OBJ_STRING))
#define OBJECT_IS_FN(obj) (object_is_type(obj, OBJ_FN))
#define OBJECT_AS_STRING(obj) (((struct object_str *)VAL_AS_OBJ(obj)))
#define OBJECT_AS_FN(obj) (((struct object_fn *)VAL_AS_OBJ(obj)))
#define OBJECT_AS_CSTRING(obj) (((struct object_str *)VAL_AS_OBJ(obj))->chars)

/**
 * @brief returns whether the passed value is a given object type
 *
 * @param value the value to check
 * @param type the type to compare
 * @return true the value is of type type
 * @return false the value is not of type type
 */
static inline bool object_is_type(lox_val_t value, enum object_type type)
{
	return VAL_IS_OBJ(value) && VAL_AS_OBJ(value)->type == type;
}

/**
 * @brief frees the passed object pointer
 *
 * @param obj the object to free
 */
void object_free(struct object *obj);

/**
 * @brief creates a new lox function object
 *
 * @return struct object_fn* the newly allocated lox function object
 */
struct object_fn *object_fn_new();

/**
 * @brief creates a new lox string object
 *
 * @param chars string chars
 * @param len length of the string
 * @return struct object_str* the newly allocated string object
 */
struct object_str *object_str_new(const char *chars, size_t len);

/**
 * @brief concats the two strings in to a new lox string object
 *
 * @param a lox string a
 * @param b lox string b
 * @return struct object_str* the newly allocated string object
 */
struct object_str *object_str_concat(const struct object_str *a,
				     const struct object_str *b);

/**
 * @brief whether the two passed objects are equal
 *
 * @param a object a
 * @param b object b
 * @return true they are equal
 * @return false they are not equal
 */
bool object_equals(const struct object *a, const struct object *b);

/**
 * @brief prints the passed object
 *
 * @param val the object to print
 */
void object_print(lox_val_t val);

/**
 * @brief generates a hash value for the passed string
 *
 * @param str the string to hash
 * @return hash_t the hash value
 */
hash_t obj_str_gen_hash(const struct object_str *str);

#endif // __CLOX_OBJECT_FUNC_H__