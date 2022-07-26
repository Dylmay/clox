#ifndef __CLOX_UTIL_VALUE_H__
#define __CLOX_UTIL_VALUE_H__

#include "util/common.h"
#include "util/list/list.h"
#include "object.h"

//! @brief lox value types
enum value_type {
	VAL_NIL,
	VAL_BOOL,
	VAL_NUMBER,
	VAL_OBJ,
};

//! @brief lox number
typedef double lox_num_t;
//! @brief lox boolean
typedef bool lox_bool_t;
//! @brief lox base object
typedef struct object lox_obj_t;
//! @brief lox string object
typedef struct object_str lox_str_t;
//! @brief lox function object
typedef struct object_fn lox_fn_t;

//! @brief lox base value
typedef struct {
	enum value_type type;
	union {
		lox_bool_t boolean;
		lox_num_t number;
		lox_obj_t *obj;
	} as;
} lox_val_t;

#endif // __CLOX_UTIL_VALUE_H__