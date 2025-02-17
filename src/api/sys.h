/**
 * @file sys.h
 * @author Dylan Mayor
 * @brief default system imports
 *
 */
#ifndef __CLOX_API_SYS_H__
#define __CLOX_API_SYS_H__

#include "compiler/import.h"

#define INPUT_BUF_SZ 256

/**
 * @brief creates a new function definition with the given function name
 *
 */
#define CREATE_FUNC_DEF(func_name, func_def)                                   \
	{                                                                      \
		.fn_name = func_name,                                          \
		.name_sz = sizeof(func_name) - 1,                              \
		.fn = &func_def,                                               \
	}

/**
 * @brief gets the import list delivered by the sys api
 *
 * @return native_import_list_t the import list
 */
native_import_list_t sys_get_import_list();

#endif // __CLOX_API_SYS_H__