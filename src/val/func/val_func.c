#include "object_func.h"
#include "val_func.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

void val_print(lox_val_t val)
{
	switch (val.type) {
	case VAL_BOOL:
		printf(VAL_AS_BOOL(val) ? "true" : "false");
		break;

	case VAL_NIL:
		printf("nil");
		break;

	case VAL_NUMBER:
		printf("%.15g", VAL_AS_NUMBER(val));
		break;
	case VAL_OBJ:
		object_print(val);
		break;
	default:
		assert(("Unknown val type on val print", 0));
		break;
	}
}

bool val_is_falsey(lox_val_t val)
{
	switch (val.type) {
	case VAL_NUMBER:
		return !VAL_AS_NUMBER(val);
	case VAL_NIL:
		return true;
	case VAL_BOOL:
		return !VAL_AS_BOOL(val);
	default:
		assert(("Unknown value type (val_is_falsey)", 0));
		return false;
	}
}

bool val_equals(lox_val_t a, lox_val_t b)
{
	if (a.type != b.type) {
		return false;
	}

	switch (a.type) {
	case VAL_BOOL:
		return a.as.boolean == b.as.boolean;

	case VAL_NIL:
		return true;

	case VAL_NUMBER:
		return a.as.number == b.as.number;

	case VAL_OBJ:
		return object_equals(VAL_AS_OBJ(a), VAL_AS_OBJ(b));

	default:
		assert(("Unexpected val comparison type", 0));
		return false;
	}
}