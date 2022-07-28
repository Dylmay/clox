/**
 * @file object.h
 * @author Dylan Mayor
 * @brief header file for lox objects
 *
 */
#ifndef __CLOX_VAL_OBJECT_H__
#define __CLOX_VAL_OBJECT_H__

#include "util/common.h"
#include "chunk/chunk.h"

//! @brief object types
enum object_type {
	OBJ_STRING,
	OBJ_FN,
};

//! @brief base lox object
struct object {
	enum object_type type;
};

//! @brief lox string object
struct object_str {
	struct object obj;
	size_t len;
	char chars[];
};

//! @brief lox function object
struct object_fn {
	struct object obj;
	int arity;
	struct chunk chunk;
	struct object_str *name;
};

#endif // __CLOX_VAL_OBJECT_H__