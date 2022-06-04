#ifndef __CLOX_VAL_OBJECT_H__
#define __CLOX_VAL_OBJECT_H__

#include "util/common.h"

enum object_type {
	OBJ_STRING,
};

struct object {
	enum object_type type;
	struct object *next;
};

struct object_str {
	struct object obj;
	size_t len;
	char chars[];
};

#endif // __CLOX_VAL_OBJECT_H__