#ifndef __CLOX_OPS_H__
#define __CLOX_OPS_H__

#define CONST_LONG_SZ (sizeof(char) * 3)
#define CONST_LONG_MASK (0x00ffffff)

#include "chunk/chunk.h"

typedef enum {
	OP_CONSTANT,
	OP_CONSTANT_LONG,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_NEGATE,
	OP_RETURN
} op_code_t;

void op_write_const(chunk_t *chunk, lox_val_t const_val, int line);

void op_write_negate(chunk_t *chunk, int line);

void op_write_add(chunk_t *chunk, int line);
void op_write_sub(chunk_t *chunk, int line);
void op_write_mult(chunk_t *chunk, int line);
void op_write_div(chunk_t *chunk, int line);

void op_write_return(chunk_t *chunk, int line);

#endif // __CLOX_OPS_H__
