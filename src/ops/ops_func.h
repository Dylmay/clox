#ifndef __CLOX_COMPILER_OPS_FUNC_H__
#define __CLOX_COMPILER_OPS_FUNC_H__

#include <assert.h>

#include "ops.h"
#include "chunk/chunk.h"

static inline void __extended_op(chunk_t *chunk, op_code_t short_op,
				 op_code_t long_op, uint32_t offset, int line)
{
	if (offset <= UINT8_MAX) {
		chunk_write_code(chunk, short_op, line);
		chunk_write_code(chunk, (code_t)offset, line);
	} else {
		assert(("Max code offset is 256 * 24", offset < EXT_CODE_MAX));

		chunk_write_code_bulk(chunk, long_op, line, &offset,
				      EXT_CODE_SZ);
	}
}

static inline void OP_CONST_WRITE(chunk_t *chunk, lox_val_t const_val, int line)
{
	uint32_t const_offset = chunk_write_const(chunk, const_val);
	__extended_op(chunk, OP_CONSTANT, OP_CONSTANT_LONG, const_offset, line);
}

static inline void OP_VAR_DEFINE_WRITE(chunk_t *chunk, uint32_t glbl_idx,
				       int line)
{
	__extended_op(chunk, OP_VAR_DEFINE, OP_VAR_DEFINE_LONG, glbl_idx, line);
}

static inline void OP_VAR_GET_WRITE(chunk_t *chunk, uint32_t glbl_idx, int line)
{
	__extended_op(chunk, OP_VAR_GET, OP_VAR_GET_LONG, glbl_idx, line);
}

static inline void OP_VAR_SET_WRITE(chunk_t *chunk, uint32_t glbl_idx, int line)
{
	__extended_op(chunk, OP_VAR_SET, OP_VAR_SET_LONG, glbl_idx, line);
}

static inline void OP_POP_COUNT_WRITE(chunk_t *chunk, uint32_t cnt, int line)
{
	chunk_write_code(chunk, OP_POP_COUNT, line);
	chunk_write_code(chunk, (code_t)cnt, line);
}

static inline int __jump_instr_write(chunk_t *chunk, code_t jump_code, int line)
{
	chunk_write_code(chunk, jump_code, line);
	chunk_reserve_code(chunk, 2);

	return (int)chunk_cur_ip(chunk) - 2;
}

static inline int OP_JUMP_IF_FALSE_WRITE(chunk_t *chunk, int line)
{
	return __jump_instr_write(chunk, OP_JUMP_IF_FALSE, line);
}

static inline int OP_JUMP_WRITE(chunk_t *chunk, int line)
{
	return __jump_instr_write(chunk, OP_JUMP, line);
}

static inline bool OP_JUMP_PATCH(chunk_t *chunk, size_t offset)
{
	int jump = chunk_cur_ip(chunk) - offset - 2;

	if (jump > UINT16_MAX) {
		return false;
	}

	chunk_patch_code(chunk, offset, &jump, 2);

	return true;
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
CREATE_WRITE_FUNC(OP_LESS) CREATE_WRITE_FUNC(OP_PRINT) CREATE_WRITE_FUNC(OP_POP)

#undef CREATE_WRITE_FUNC
#undef FUNC_NAME_OF

#endif // __CLOX_COMPILER_OPS_FUNC_H__