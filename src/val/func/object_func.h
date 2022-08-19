/**
 * @file object_func.h
 * @author Dylan Mayor
 * @brief header file for lox object-related functions
 *
 */
#ifndef __CLOX_OBJECT_FUNC_H__
#define __CLOX_OBJECT_FUNC_H__

#include "val/val.h"
#include "val_func.h"
#include "util/map/hash.h"

/**
 * @brief Gets the object type of the given lox value
 *
 * @see enum object_type
 *
 * @param obj lox_val_t struct that is known to contain a lox object
 *
 * @return enum object_type The type of object
 *
 */
#define OBJECT_TYPE(obj) (VAL_AS_OBJ(obj)->type)

/**
 * @brief Checks whether the passed lox value is a string
 *
 * @see lox_str_t
 *
 * @param obj lox_val_t struct
 *
 * @return true lox_val_t is an object and is a string
 * @return false lox_val_t is not an object and is not an object string
 *
 */
#define OBJECT_IS_STRING(obj) (object_is_type(obj, OBJ_STRING))

/**
 * @brief Checks whether the passed lox value is a function
 *
 * @see lox_fn_t
 *
 * @return true lox_val_t is an object and is a function
 * @return false lox_val_t is not an object and is not a function
 *
 */
#define OBJECT_IS_FN(obj) (object_is_type(obj, OBJ_FN))

/**
 * @brief Checks whether the passed lox value is a native function
 *
 * @see lox_fn_t
 *
 * @return true lox_val_t is an object and is a native function
 * @return false lox_val_t is not an object and is not a native function
 *
 */
#define OBJECT_IS_NATIVE(obj) (object_is_type(obj, OBJ_NATIVE))

/**
 * @brief Checks whether the passed lox value is a closure
 *
 * @see lox_fn_t
 *
 * @return true lox_val_t is an object and is a closure
 * @return false lox_val_t is not an object and is not a closure
 *
 */
#define OBJECT_IS_CLOSURE(obj) (object_is_type(obj, OBJ_CLOSURE))

/**
 * @brief returns the the passed lox value as a lox string. Undefined behaviour if the value is not a lox object and is not lox string
 *
 * @see lox_str_t
 *
 * @param obj the lox value to get as a lox string
 *
 * @return struct object_str * lox string
 */
#define OBJECT_AS_STRING(obj) (((struct object_str *)VAL_AS_OBJ(obj)))

/**
 * @brief returns the passed lox value as a lox function. Undefined behaviour if the value is not a lox object and is not a lox function
 *
 * @see lox_fn_t
 *
 * @param obj the lox value to get as a lox function
 *
 * @return struct object_fn * lox function
 *
 */
#define OBJECT_AS_FN(obj) (((struct object_fn *)VAL_AS_OBJ(obj)))

/**
 * @brief returns the passed lox value as a lox native function. Undefined behaviour if the value is not a lox object and is not a lox native function
 *
 * @see lox_fn_t
 *
 * @param obj the lox value to get as a lox function
 *
 * @return struct object_fn * lox function
 *
 */
#define OBJECT_AS_NATIVE(obj) (((struct object_native_fn *)VAL_AS_OBJ(obj)))

/**
 * @brief returns the passed lox value as a lox closure. Undefined behaviour if the value is not a lox object and is not a lox closure
 *
 * @see lox_fn_t
 *
 * @param obj the lox value to get as a lox closure
 *
 * @return struct object_closure * lox closure
 *
 */
#define OBJECT_AS_CLOSURE(obj) (((struct object_closure *)VAL_AS_OBJ(obj)))

/**
 * @brief returns the passed lox value as a lox string and gets the c string (asciiz) associated with it.
 * Undefined behaviour if the value is not a lox object and is not a lox function
 *
 * @see lox_str_t
 *
 * @param obj the lox value to get the c string from
 *
 * @return char* c string of the passed lox value
 *
 */
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
 * @brief generates a hash value for the passed string
 *
 * @param str the string to hash
 * @return hash_t the hash value
 */
hash_t obj_str_gen_hash(const struct object_str *str);

/**
 * @brief wraps the given native function in to a callable lox native function
 *
 * @param native_fn the native function to wrap
 * @return struct object_native_fn* the wrapped function
 */
struct object_native_fn *object_native_fn_new(native_fn native_fn);

/**
 * @brief creates a new closure over the given function
 *
 * @param fn the function to wrap
 * @return struct object_closure* the new object closure
 */
struct object_closure *object_closure_new(struct object_fn *fn);

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

#endif // __CLOX_OBJECT_FUNC_H__