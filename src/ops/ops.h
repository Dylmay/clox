#ifndef __CLOX_OPS_H__
#define __CLOX_OPS_H__

#include "chunk/chunk.h"

typedef enum { OP_CONSTANT, OP_CONSTANT_LONG, OP_RETURN } op_code_t;

void op_write_const(chunk_t *chunk, lox_val_t const_val, int line);

void op_write_return(chunk_t *chunk, int line);

#endif // __CLOX_OPS_H__
