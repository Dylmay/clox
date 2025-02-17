/**
 * @file import.h
 * @author Dylan Mayor
 * @brief import api for lox objects and native functions
 *
 */
#ifndef __CLOX_API_IMPORT_H__
#define __CLOX_API_IMPORT_H__

#include "val/val.h"

//! @brief import list struct
typedef struct {
	const char *import_name;
	size_t import_cnt;
	native_import_t *import_arr;
} native_import_list_t;

#endif // __CLOX_API_IMPORT_H__