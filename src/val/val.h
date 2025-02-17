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
#include "state/lookup/lookup.h"
#include "val_type.h"

//! @brief lox number
typedef double lox_num_t;
//! @brief lox boolean
typedef bool lox_bool_t;

//! @brief base lox object
typedef struct {
	enum object_type type;
} lox_obj_t;

//! @brief lox base value
typedef struct {
	enum value_type type;
	union {
		lox_bool_t boolean;
		lox_num_t number;
		lox_obj_t *obj;
	} as;
} lox_val_t;

//! @brief lox string object
typedef struct {
	lox_obj_t obj;
	size_t len;
	char chars[];
} lox_str_t;

//! @brief lox function object
typedef struct {
	lox_obj_t obj;
	int arity;
	uint32_t upval_cnt;
	chunk_t chunk;
	lox_str_t *name;
} lox_fn_t;

typedef lox_val_t (*native_fn)(int arg_cnt, lox_val_t *args);

//! @brief native function import
typedef struct {
	const char *fn_name;
	size_t name_sz;
	native_fn fn;
} native_import_t;

typedef struct {
	lox_obj_t obj;
	native_import_t import;
} lox_native_t;

typedef struct {
	lox_obj_t obj;
	list_t upvalues;
	lox_fn_t *fn;
} lox_closure_t;

typedef struct {
	lox_obj_t obj;
	lox_str_t *name;
	struct {
		lookup_t table;
		uint32_t idx;
	} field_lookup;
	struct {
		lookup_t table;
		uint32_t idx;
	} static_lookup;
	list_t statics;
} lox_class_t;

typedef struct {
	lox_obj_t obj;
	lox_class_t *cls;
	list_t fields;
} lox_instance_t;

typedef struct lox_upval_t {
	lox_obj_t obj;
	lox_val_t *location;
	lox_val_t closed;
	struct lox_upval_t *next;
} lox_upval_t;

#endif // __CLOX_UTIL_VALUE_H__