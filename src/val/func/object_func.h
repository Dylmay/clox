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
 * @return lox_str_t * lox string
 */
#define OBJECT_AS_STRING(obj) (((lox_str_t *)VAL_AS_OBJ(obj)))

/**
 * @brief returns the passed lox value as a lox function. Undefined behaviour if the value is not a lox object and is not a lox function
 *
 * @see lox_fn_t
 *
 * @param obj the lox value to get as a lox function
 *
 * @return lox_fn_t * lox function
 *
 */
#define OBJECT_AS_FN(obj) (((lox_fn_t *)VAL_AS_OBJ(obj)))

/**
 * @brief returns the passed lox value as a lox class. Undefined behaviour if the value is not a lox object and is not a lox class instance
 *
 * @see lox_instance_t
 *
 * @param obj the lox value to get as a lox class instance
 *
 * @return lox_instance_t * lox class instance
 *
 */
#define OBJECT_AS_INSTANCE(obj) (((lox_instance_t *)VAL_AS_OBJ(obj)))

/**
 * @brief returns the passed lox value as a lox class. Undefined behaviour if the value is not a lox object and is not a lox class
 *
 * @see lox_class_t
 *
 * @param obj the lox value to get as a lox class
 *
 * @return lox_class_t * lox class
 *
 */
#define OBJECT_AS_CLASS(obj) (((lox_class_t *)VAL_AS_OBJ(obj)))

/**
 * @brief returns the passed lox value as a lox native function. Undefined behaviour if the value is not a lox object and is not a lox native function
 *
 * @see lox_fn_t
 *
 * @param obj the lox value to get as a lox function
 *
 * @return lox_native_t * lox function
 *
 */
#define OBJECT_AS_NATIVE(obj) (((lox_native_t *)VAL_AS_OBJ(obj)))

/**
 * @brief returns the passed lox value as a lox closure. Undefined behaviour if the value is not a lox object and is not a lox closure
 *
 * @see lox_fn_t
 *
 * @param obj the lox value to get as a lox closure
 *
 * @return lox_closure_t * lox closure
 *
 */
#define OBJECT_AS_CLOSURE(obj) (((lox_closure_t *)VAL_AS_OBJ(obj)))

/**
 * @brief returns the passed lox value as a lox upvalue. Undefined behaviour if the value is not a lox object and is not a lox upvalue
 *
 * @see lox_upval_t
 *
 * @param obj the lox value to get as a lox upvalue
 *
 * @return lox_upval_t * lox upvalue
 *
 */
#define OBJECT_AS_UPVALUE(obj) (((lox_upval_t *)VAL_AS_OBJ(obj)))

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
#define OBJECT_AS_CSTRING(obj) (((lox_str_t *)VAL_AS_OBJ(obj))->chars)

/**
 * @brief creates a new object string for a given literal. i.e. "string"
 *
 * @param obj the literal string
 *
 * @return lox_str_t* the created literal string
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
 * @return lox_fn_t* the newly allocated lox function object
 */
lox_fn_t *object_fn_new(lox_str_t *name);

/**
 * @brief creates a new lox string object
 *
 * @param chars string chars
 * @param len length of the string
 * @return lox_str_t* the newly allocated string object
 */
lox_str_t *object_str_new(const char *chars, size_t len);

/**
 * @brief concats the two strings in to a new lox string object
 *
 * @param a lox string a
 * @param b lox string b
 * @return lox_str_t* the newly allocated string object
 */
lox_str_t *object_str_concat(const lox_str_t *a, const lox_str_t *b);

/**
 * @brief generates a hash value for the passed string
 *
 * @param str the string to hash
 * @return hash_t the hash value
 */
hash_t obj_str_gen_hash(const lox_str_t *str);

/**
 * @brief wraps the given native function in to a callable lox native function
 *
 * @param native_fn the native function to wrap
 * @return lox_native_t* the wrapped function
 */
lox_native_t *object_native_fn_new(const native_import_t native_import);

/**
 * @brief creates a new closure over the given function
 *
 * @param fn the function to wrap
 * @return lox_closure_t* the new object closure
 */
lox_closure_t *object_closure_new(lox_fn_t *fn);

/**
 * @brief creates a new class with the given name
 *
 * @param fn the function to wrap
 * @return lox_closure_t* the new object closure
 */
lox_class_t *object_class_new(lox_str_t *fn);

/**
 * @brief creates a new instance of the given class
 *
 * @param cls the class to instantiate
 * @return lox_instance_t* the new object instance
 */
lox_instance_t *object_instance_new(lox_class_t *cls);

/**
 * @brief gets the upvalue at the given position within the closure
 *
 * @param closure the closure to read
 * @param idx the index of the upvalue
 * @return lox_upval_t* the upvalue pointer
 */
static inline lox_upval_t *object_closure_get_upval(lox_closure_t *closure,
						    int idx)
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
static inline void object_closure_set_upval(lox_closure_t *closure, int idx,
					    lox_val_t *val)
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
static inline void object_closure_push_upval(lox_closure_t *closure,
					     lox_upval_t *val)
{
	list_push(&closure->upvalues, &val);
}

/**
 * @brief creates a new upvalue over the given lox value
 *
 * @param slot the value to wrap
 * @return lox_upval_t* the new object upvalue
 */
lox_upval_t *object_upval_new(lox_val_t *slot);

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