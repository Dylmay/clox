/**
 * @file ops_name.h
 * @author Dylan Mayor
 * @brief header file for retrieving a string op name of a given op code
 *
 * @see ops.h
 *
 */
#ifndef __OPS_NAME_H__
#define __OPS_NAME_H__

#include "ops/ops.h"

#define X(a, b) b,
//! @brief op code name table. SHOULD NOT BE ACCESSED DIRECTLY. @see op_name()
static char *op_code_names[] = {
#include "ops/ops_table.h"
};
#undef X

/**
 * @brief gets the displayable op name for a function
 *
 * @param op the op to convert
 * @return char* the string representation of the op code
 */
static inline char *op_name(op_code_t op)
{
	return op_code_names[op];
}
#endif // __OPS_NAME_H__