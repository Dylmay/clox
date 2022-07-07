#ifndef __CLOX_VAL_OBJECT_H__
#define __CLOX_VAL_OBJECT_H__

#include "util/common.h"
#include "chunk/chunk.h"

enum object_type {
	OBJ_STRING,
	OBJ_FN,
};

struct object {
	enum object_type type;
};

struct object_str {
	struct object obj;
	size_t len;
	char chars[];
};

struct object_fn {
	struct object obj;
	int arity;
	struct chunk chunk;
	struct object_str *name;
};

#endif // __CLOX_VAL_OBJECT_H__