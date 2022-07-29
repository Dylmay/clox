/**
 * @file ops_func.h
 * @author Dylan Mayor
 * @brief header file for functions related to VM opcodes
 *
 */
#ifndef __CLOX_COMPILER_OPS_FUNC_H__
#define __CLOX_COMPILER_OPS_FUNC_H__

#include <assert.h>

#include "ops.h"
#include "chunk/chunk.h"
#include "chunk/func/chunk_func.h"

static inline void __extended_op(struct chunk *chunk, op_code_t short_op,
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

static inline int __jump_instr_write(struct chunk *chunk, code_t jump_code,
				     int line)
{
	chunk_write_code(chunk, jump_code, line);
	chunk_reserve_code(chunk, 2);

	return (int)chunk_cur_ip(chunk) - 2;
}

static inline bool op_patch_jump(lox_fn_t *fn, long offset)
{
	long jump = chunk_cur_ip(&fn->chunk) - offset - 2;

	if (jump > INT16_MAX || jump < INT16_MIN) {
		return false;
	}

	chunk_patch_code(&fn->chunk, offset, &jump, 2);

	return true;
}

static inline void OP_CONST_WRITE(lox_fn_t *fn, lox_val_t const_val, int line)
{
	uint32_t const_offset = chunk_write_const(&fn->chunk, const_val);
	__extended_op(&fn->chunk, OP_CONSTANT, OP_CONSTANT_LONG, const_offset,
		      line);
}

static inline void OP_CALL_WRITE(lox_fn_t *fn, uint8_t arg_cnt, int line)
{
	chunk_write_code(&fn->chunk, OP_CALL, line);
	chunk_write_code(&fn->chunk, (code_t)arg_cnt, line);
}

static inline void OP_POP_COUNT_WRITE(lox_fn_t *fn, uint32_t cnt, int line)
{
	chunk_write_code(&fn->chunk, OP_POP_COUNT, line);
	chunk_write_code(&fn->chunk, (code_t)cnt, line);
}

static inline bool OP_LOOP_WRITE(lox_fn_t *fn, long loop_begin, int line)
{
	long jump = loop_begin - chunk_cur_ip(&fn->chunk) - 3;

	if (jump > INT16_MAX || jump < INT16_MIN) {
		return false;
	}

	chunk_write_code_bulk(&fn->chunk, OP_JUMP, line, &jump,
			      sizeof(int16_t));

	return true;
}

#define FUNC_NAME_OF(op) op##_WRITE
#define CREATE_WRITE_FUNC(instr)                                               \
	static inline void FUNC_NAME_OF(instr)(lox_fn_t * fn, int line)        \
	{                                                                      \
		chunk_write_code(&fn->chunk, instr, line);                     \
	}

#define CREATE_JUMP_FUNC(instr)                                                \
	static inline int instr##_WRITE(lox_fn_t *fn, int line)                \
	{                                                                      \
		return __jump_instr_write(&fn->chunk, instr, line);            \
	}

#define CREATE_EXTENDED_WRITE_FUNC(short_op, long_op)                          \
	static inline void short_op##_WRITE(lox_fn_t *fn, uint32_t glbl_idx,   \
					    int line)                          \
	{                                                                      \
		__extended_op(&fn->chunk, short_op, long_op, glbl_idx, line);  \
	}

CREATE_JUMP_FUNC(OP_JUMP)
CREATE_JUMP_FUNC(OP_JUMP_IF_FALSE)

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
CREATE_WRITE_FUNC(OP_MOD)

CREATE_EXTENDED_WRITE_FUNC(OP_VAR_DEFINE, OP_GLOBAL_DEFINE_LONG)
CREATE_EXTENDED_WRITE_FUNC(OP_VAR_GET, OP_GLOBAL_GET_LONG)
CREATE_EXTENDED_WRITE_FUNC(OP_VAR_SET, OP_GLOBAL_SET_LONG)
CREATE_EXTENDED_WRITE_FUNC(OP_GLOBAL_DEFINE, OP_GLOBAL_DEFINE_LONG)
CREATE_EXTENDED_WRITE_FUNC(OP_GLOBAL_GET, OP_GLOBAL_GET_LONG)
CREATE_EXTENDED_WRITE_FUNC(OP_GLOBAL_SET, OP_GLOBAL_SET_LONG)

static inline void OP_BANG_EQ_WRITE(lox_fn_t *fn, int line)
{
	OP_EQUAL_WRITE(fn, line);
	OP_NOT_WRITE(fn, line);
}

static inline void OP_GREATER_EQ_WRITE(lox_fn_t *fn, int line)
{
	OP_LESS_WRITE(fn, line);
	OP_NOT_WRITE(fn, line);
}

static inline void OP_LESS_EQ_WRITE(lox_fn_t *fn, int line)
{
	OP_GREATER_WRITE(fn, line);
	OP_NOT_WRITE(fn, line);
}

#undef CREATE_JUMP_FUNC
#undef CREATE_EXTENDED_WRITE_FUNC
#undef CREATE_WRITE_FUNC
#undef FUNC_NAME_OF

#endif // __CLOX_COMPILER_OPS_FUNC_H__