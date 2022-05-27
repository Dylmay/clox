#ifndef __CLOX_UTIL_VALUE_H__
#define __CLOX_UTIL_VALUE_H__

#include "common.h"
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

#define BOOL_VAL(value) ((lox_val_t){ VAL_BOOL, { .boolean = value } })
#define NIL_VAL() ((lox_val_t){ VAL_NIL, { .number = 0 } })
#define NUMBER_VAL(value) ((lox_val_t){ VAL_NUMBER, { .number = value } })

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)

#define PRINT_VAL(value) (printf("%g", AS_NUMBER(value)))

#endif // __CLOX_UTIL_VALUE_H__