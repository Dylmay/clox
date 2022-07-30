/**
 * @file import.h
 * @author Dylan Mayor
 * @brief import api for lox objects and native functions
 *
 */
#ifndef __CLOX_API_IMPORT_H__
#define __CLOX_API_IMPORT_H__

#include "val/val.h"

//! @brief native function import
struct native_import {
	const char *fn_name;
	size_t name_sz;
	native_fn fn;
};

//! @brief import list struct
struct import_list {
	const char *import_name;
	size_t import_cnt;
	struct native_import *import_arr;
};

#endif // __CLOX_API_IMPORT_H__