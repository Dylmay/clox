/**
 * @file val.h
 * @author Dylan Mayor
 * @brief header file for lox value types
 *
 * @see object.h for definitions of lox objects
 *
 */
#ifndef __CLOX_UTIL_VALUE_H__
#define __CLOX_UTIL_VALUE_H__

#include "util/common.h"
#include "util/list/list.h"
#include "chunk/chunk.h"

//! @brief lox value types
enum value_type {
	VAL_NIL,
	VAL_BOOL,
	VAL_NUMBER,
	VAL_OBJ,
	VAL_ERR,
};

//! @brief object types
enum object_type {
	OBJ_STRING,
	OBJ_FN,
	OBJ_NATIVE,
	OBJ_CLOSURE,
};

//! @brief lox number
typedef double lox_num_t;
//! @brief lox boolean
typedef bool lox_bool_t;

//! @brief base lox object
typedef struct object {
	enum object_type type;
} lox_obj_t;

//! @brief lox base value
typedef struct __lox_val {
	enum value_type type;
	union {
		lox_bool_t boolean;
		lox_num_t number;
		lox_obj_t *obj;
	} as;
} lox_val_t;

//! @brief lox string object
typedef struct object_str {
	struct object obj;
	size_t len;
	char chars[];
} lox_str_t;

//! @brief lox function object
typedef struct object_fn {
	struct object obj;
	int arity;
	chunk_t chunk;
	struct object_str *name;
} lox_fn_t;

typedef lox_val_t (*native_fn)(int arg_cnt, lox_val_t *args);
typedef struct object_native_fn {
	struct object obj;
	native_fn fn;
} lox_native_t;

typedef struct object_closure {
	struct object obj;
	lox_fn_t *fn;
} lox_closure_t;

#endif // __CLOX_UTIL_VALUE_H__