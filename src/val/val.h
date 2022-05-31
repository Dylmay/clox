#ifndef __CLOX_UTIL_VALUE_H__
#define __CLOX_UTIL_VALUE_H__

#include <stdio.h>

#include "util/common.h"
#include "util/list/list.h"

enum value_type { VAL_BOOL, VAL_NIL, VAL_NUMBER };

typedef double lox_num_t;
typedef bool lox_bool_t;

typedef struct {
	enum value_type type;
	union {
		bool boolean;
		double number;
	} as;
} lox_val_t;

#endif // __CLOX_UTIL_VALUE_H__