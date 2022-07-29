/**
 * @file ops.h
 * @author Dylan Mayor
 * @brief header file for vm op functions and related data
 *
 */
#ifndef __CLOX_OPS_H__
#define __CLOX_OPS_H__

//! @brief size of an extended op code
#define EXT_CODE_SZ (sizeof(uint8_t) * 3)
//! @brief mask for retrieving the extended op code
#define EXT_CODE_MASK (0x00ffffff)
//! @brief max value that an extended op code can hold
#define EXT_CODE_MAX (UINT8_MAX * 24)

#define X(a, b) a,
//! @brief enum of operations the VM can perform
typedef enum __opcode {
#include "ops_table.h"
} op_code_t;
#undef X

#endif // __CLOX_OPS_H__
