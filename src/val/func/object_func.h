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
 * @return false lox_val_t is not an object or is not an object string
 *
 */
#define OBJECT_IS_STRING(obj) (object_is_type(obj, OBJ_STRING))

/**
 * @brief Checks whether the passed lox value is a function
 *
 * @see lox_fn_t
 *
 * @return true lox_val_t is an object and is a function
 * @return false lox_val_t is not an object or is not a function
 *
 */
#define OBJECT_IS_FN(obj) (object_is_type(obj, OBJ_FN))

/**
 * @brief Checks whether the passed lox value is a class instance
 *
 * @see lox_instance_t
 *
 * @return true lox_instance_t is an object and is a function
 * @return false lox_instance_t is not an object or is not a function
 *
 */
#define OBJECT_IS_INSTANCE(obj) (object_is_type(obj, OBJ_INSTANCE))

/**
 * @brief Checks whether the passed lox value is a class
 *
 * @see lox_class_t
 *
 * @return true lox_val_t is an object and is a class
 * @return false lox_val_t is not an object or is not a class
 *
 */
#define OBJECT_IS_CLASS(obj) (object_is_type(obj, OBJ_CLASS))

/**
 * @brief Checks whether the passed lox value is a native function
 *
 * @see lox_native_t
 *
 * @return true lox_val_t is an object and is a native function
 * @return false lox_val_t is not an object or is not a native function
 *
 */
#define OBJECT_IS_NATIVE(obj) (object_is_type(obj, OBJ_NATIVE))

/**
 * @brief Checks whether the passed lox value is a closure
 *
 * @see lox_fn_t
 *
 * @return true lox_val_t is an object and is a closure
 * @return false lox_val_t is not an object or is not a closure
 *
 */
#define OBJECT_IS_CLOSURE(obj) (object_is_type(obj, OBJ_CLOSURE))

/**
 * @brief Checks whether the passed lox value is an upvalue
 *
 * @see lox_upval_t
 *
 * @return true lox_upval_t is an object and is an upval
 * @return false lox_upval_t is not an object or is not an upval
 *
 */
#define OBJECT_IS_UPVALUE(obj) (object_is_type(obj, OBJ_UPVALUE))

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
 * @brief returns the passed lox value as a lox class. Undefined behaviour if the value is not a lox object and is not a lox class instance
 *
 * @see lox_instance_t
 *
 * @param obj the lox value to get as a lox class instance
 *
 * @return struct object_instance * lox class instance
 *
 */
#define OBJECT_AS_INSTANCE(obj) (((struct object_instance *)VAL_AS_OBJ(obj)))

/**
 * @brief returns the passed lox value as a lox class. Undefined behaviour if the value is not a lox object and is not a lox class
 *
 * @see lox_class_t
 *
 * @param obj the lox value to get as a lox class
 *
 * @return struct object_class * lox class
 *
 */
#define OBJECT_AS_CLASS(obj) (((struct object_class *)VAL_AS_OBJ(obj)))

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
 * @brief returns the passed lox value as a lox upvalue. Undefined behaviour if the value is not a lox object and is not a lox upvalue
 *
 * @see lox_upval_t
 *
 * @param obj the lox value to get as a lox upvalue
 *
 * @return struct object_upval * lox upvalue
 *
 */
#define OBJECT_AS_UPVALUE(obj) (((struct object_upval *)VAL_AS_OBJ(obj)))

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
 * @brief creates a new object string for a given literal. i.e. "string"
 *
 * @param obj the literal string
 *
 * @return struct object_str* the created literal string
 */
#define LITERAL_OBJECT_STRING(str) (object_str_new(str, sizeof(str) - 1))

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
 * @brief creates a new class with the given name
 *
 * @param fn the function to wrap
 * @return struct object_closure* the new object closure
 */
struct object_class *object_class_new(struct object_str *fn);

/**
 * @brief creates a new instance of the given class
 *
 * @param cls the class to instantiate
 * @return struct object_instance* the new object instance
 */
struct object_instance *object_instance_new(struct object_class *cls);

/**
 * @brief gets the upvalue at the given position within the closure
 *
 * @param closure the closure to read
 * @param idx the index of the upvalue
 * @return lox_upval_t* the upvalue pointer
 */
static inline lox_upval_t *
object_closure_get_upval(struct object_closure *closure, int idx)
{
	return *(lox_upval_t **)list_get(&closure->upvalues, idx);
}

/**
 * @brief sets the given upvalue with the chosen value
 *
 * @param closure the closure to write to
 * @param idx the index of the upvalue
 * @param val the new value
 */
static inline void object_closure_set_upval(struct object_closure *closure,
					    int idx, lox_val_t *val)
{
	lox_upval_t *upval = object_closure_get_upval(closure, idx);
	*upval->location = *val;
}

/**
 * @brief pushes the given value on to the upvalue list
 *
 * @param closure the closure to write to
 * @param val the new upvalue value
 */
static inline void object_closure_push_upval(struct object_closure *closure,
					     lox_upval_t *val)
{
	list_push(&closure->upvalues, &val);
}

/**
 * @brief creates a new upvalue over the given lox value
 *
 * @param slot the value to wrap
 * @return struct object_upval* the new object upvalue
 */
struct object_upval *object_upval_new(lox_val_t *slot);

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
 * @brief returns a string representation of the object
 *
 * @param val the object to convert
 * @return lox_val_t the string object
 */
lox_val_t object_to_string(lox_val_t val);

#endif // __CLOX_OBJECT_FUNC_H__