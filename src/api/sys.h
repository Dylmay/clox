/**
 * @file sys.h
 * @author Dylan Mayor
 * @brief default system imports
 *
 */
#ifndef __CLOX_API_SYS_H__
#define __CLOX_API_SYS_H__

#include "compiler/import.h"

#define INPUT_BUF_SZ 1024

/**
 * @brief gets the import list delivered by the sys api
 *
 * @return struct import_list the import list
 */
struct import_list sys_get_import_list();

#endif // __CLOX_API_SYS_H__