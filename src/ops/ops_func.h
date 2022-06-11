#ifndef __CLOX_COMPILER_OPS_FUNC_H__
#define __CLOX_COMPILER_OPS_FUNC_H__

#include <assert.h>

#include "ops.h"
#include "chunk/chunk.h"

static inline void OP_CONST_WRITE(chunk_t *chunk, lox_val_t const_val, int line)
{
	size_t const_offset = chunk_write_const(chunk, const_val);
	
	if (const_offset <= UINT8_MAX) {
		chunk_write_code(chunk, OP_CONSTANT, line);
		chunk_write_code(chunk, (code_t)const_offset, line);
	} else {
		assert(("Max number of consts is 256 * 24",
			const_offset < 256 * 24));

		chunk_write_code_bulk(chunk, OP_CONSTANT_LONG, line,
				      &const_offset, 3);
	}
}

#define FUNC_NAME_OF(op) op##_WRITE
#define CREATE_WRITE_FUNC(instr)                                               \
	static inline void FUNC_NAME_OF(instr)(chunk_t * chunk, int line)      \
	{                                                                      \
		chunk_write_code(chunk, instr, line);                          \
	}

CREATE_WRITE_FUNC(OP_ADD)
CREATE_WRITE_FUNC(OP_FALSE)
CREATE_WRITE_FUNC(OP_TRUE)
CREATE_WRITE_FUNC(OP_NIL)
CREATE_WRITE_FUNC(OP_NOT)
CREATE_WRITE_FUNC(OP_SUBTRACT)
CREATE_WRITE_FUNC(OP_MULTIPLY)
CREATE_WRITE_FUNC(OP_DIVIDE)
CREATE_WRITE_FUNC(OP_NEGATE)
CREATE_WRITE_FUNC(OP_RETURN)
CREATE_WRITE_FUNC(OP_EQUAL)
CREATE_WRITE_FUNC(OP_GREATER)
CREATE_WRITE_FUNC(OP_LESS)
CREATE_WRITE_FUNC(OP_PRINT)
CREATE_WRITE_FUNC(OP_POP)

#undef CREATE_WRITE_FUNC
#undef FUNC_NAME_OF

#endif // __CLOX_COMPILER_OPS_FUNC_H__