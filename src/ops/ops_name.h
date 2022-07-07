#ifndef __OPS_NAME_H__
#define __OPS_NAME_H__

#include "ops/ops.h"

#define X(a, b) b,
static char *op_code_names[] = {
#include "ops/ops_table.h"
};
#undef X

static inline char *op_name(op_code_t op)
{
	return op_code_names[op];
}
#endif // __OPS_NAME_H__