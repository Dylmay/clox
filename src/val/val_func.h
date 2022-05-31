#ifndef __CLOX_VAL_FUNC_H__
#define __CLOX_VAL_FUNC_H__

#include "val.h"

#define VAL_CREATE_BOOL(value) ((lox_val_t){ VAL_BOOL, { .boolean = value } })
#define VAL_CREATE_NIL ((lox_val_t){ VAL_NIL, { .number = 0 } })
#define VAL_CREATE_NUMBER(value)                                               \
	((lox_val_t){ VAL_NUMBER, { .number = value } })

#define VAL_AS_BOOL(value) ((value).as.boolean)
#define VAL_AS_NUMBER(value) ((value).as.number)

#define VAL_IS_BOOL(value) ((value).type == VAL_BOOL)
#define VAL_IS_NIL(value) ((value).type == VAL_NIL)
#define VAL_IS_NUMBER(value) ((value).type == VAL_NUMBER)

static inline bool val_is_falsey(lox_val_t val)
{
	return VAL_IS_NIL(val) || (VAL_IS_BOOL(val) && !VAL_AS_BOOL(val));
}

static inline void val_print(lox_val_t val)
{
	switch (val.type) {
	case VAL_BOOL:
		printf(VAL_AS_BOOL(val) ? "true" : "false");
		break;

	case VAL_NIL:
		printf("nil");
		break;

	case VAL_NUMBER:
		printf("%g", VAL_AS_NUMBER(val));
		break;
	}
}

#endif // __CLOX_VAL_FUNC_H__