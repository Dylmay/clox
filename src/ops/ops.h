#ifndef __CLOX_OPS_H__
#define __CLOX_OPS_H__

#define EXT_CODE_SZ (sizeof(char) * 3)
#define EXT_CODE_MASK (0x00ffffff)
#define EXT_CODE_MAX (UINT8_MAX * 24)

#define X(a, b) a,
typedef enum {
#include "ops_table.h"
} op_code_t;
#undef X

#endif // __CLOX_OPS_H__
