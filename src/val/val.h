#ifndef __CLOX_UTIL_VALUE_H__
#define __CLOX_UTIL_VALUE_H__

#include "util/common.h"
#include "util/list/list.h"
#include "object.h"

enum value_type { VAL_BOOL, VAL_NIL, VAL_NUMBER, VAL_OBJ };

typedef double lox_num_t;
typedef bool lox_bool_t;
typedef struct object lox_obj_t;
typedef struct object_str lox_str_t;
typedef struct object_fn lox_fn_t;

typedef struct {
	enum value_type type;
	union {
		lox_bool_t boolean;
		lox_num_t number;
		lox_obj_t *obj;
	} as;
} lox_val_t;

#endif // __CLOX_UTIL_VALUE_H__