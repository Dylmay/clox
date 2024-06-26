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

static inline void __extended_op(chunk_t *chunk, op_code_t short_op,
				 op_code_t long_op, uint32_t offset,
				 uint32_t line)
{
	if (offset <= UINT8_MAX) {
		chunk_write_code(chunk, (code_t)short_op, line);
		chunk_write_code(chunk, (code_t)offset, line);
	} else {
		assert(("Max code offset is 256 * 24", offset < EXT_CODE_MAX));

		chunk_write_code_extended(chunk, (code_t)long_op, line, &offset,
					  EXT_CODE_SZ);
	}
}

static inline long __jump_instr_write(chunk_t *chunk, code_t jump_code,
				      uint32_t line)
{
	chunk_write_code(chunk, jump_code, line);
	chunk_reserve_code(chunk, 2);

	return chunk_cur_instr(chunk) - 2;
}

static inline bool op_patch_jump(lox_fn_t *fn, long offset)
{
	long jump = chunk_cur_instr(&fn->chunk) - offset - 2;

	if (jump > INT16_MAX || jump < INT16_MIN) {
		return false;
	}

	chunk_patch_code(&fn->chunk, offset, &jump, 2);

	return true;
}

static inline void OP_CONST_WRITE(lox_fn_t *fn, lox_val_t const_val,
				  uint32_t line)
{
	uint32_t const_offset =
		(uint32_t)chunk_write_const(&fn->chunk, const_val);
	__extended_op(&fn->chunk, OP_CONSTANT, OP_CONSTANT_LONG, const_offset,
		      line);
}

static inline void OP_CLOSURE_WRITE(lox_fn_t *fn, lox_val_t const_val,
				    uint32_t line)
{
	uint32_t const_offset = chunk_write_const(&fn->chunk, const_val);
	__extended_op(&fn->chunk, OP_CLOSURE, OP_CLOSURE_LONG, const_offset,
		      line);
}

static inline void OP_CALL_WRITE(lox_fn_t *fn, uint8_t arg_cnt, uint32_t line)
{
	chunk_write_code(&fn->chunk, OP_CALL, line);
	chunk_write_code(&fn->chunk, (code_t)arg_cnt, line);
}

static inline void OP_POP_COUNT_WRITE(lox_fn_t *fn, uint32_t cnt, uint32_t line)
{
	chunk_write_code(&fn->chunk, OP_POP_COUNT, line);
	chunk_write_code(&fn->chunk, (code_t)cnt, line);
}

static inline bool OP_LOOP_WRITE(lox_fn_t *fn, long loop_begin, uint32_t line)
{
	long jump = loop_begin - chunk_cur_instr(&fn->chunk) - 3;

	if (jump > INT16_MAX || jump < INT16_MIN) {
		return false;
	}

	chunk_write_code_extended(&fn->chunk, OP_JUMP, line, &jump,
				  sizeof(int16_t));

	return true;
}

static inline void OP_PROPERTY_GET_WRITE(lox_fn_t *fn, lox_val_t prop_name,
					 uint32_t line)
{
	size_t const_offset = chunk_write_const(&fn->chunk, prop_name);
	__extended_op(&fn->chunk, OP_PROPERTY_GET, OP_PROPERTY_GET_LONG,
		      const_offset, line);
}

static inline void OP_PROPERTY_SET_WRITE(lox_fn_t *fn, lox_val_t prop_name,
					 uint32_t line)
{
	size_t const_offset = chunk_write_const(&fn->chunk, prop_name);
	__extended_op(&fn->chunk, OP_PROPERTY_SET, OP_PROPERTY_SET_LONG,
		      const_offset, line);
}

#define FUNC_NAME_OF(op) op##_WRITE
#define CREATE_WRITE_FUNC(instr)                                               \
	static inline void FUNC_NAME_OF(instr)(lox_fn_t * fn, uint32_t line)   \
	{                                                                      \
		chunk_write_code(&fn->chunk, instr, line);                     \
	}

#define CREATE_JUMP_FUNC(instr)                                                \
	static inline size_t instr##_WRITE(lox_fn_t *fn, uint32_t line)        \
	{                                                                      \
		return __jump_instr_write(&fn->chunk, instr, line);            \
	}

#define CREATE_EXTENDED_WRITE_FUNC(short_op, long_op)                          \
	static inline void short_op##_WRITE(lox_fn_t *fn, uint32_t glbl_idx,   \
					    uint32_t line)                     \
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
CREATE_WRITE_FUNC(OP_POP)
CREATE_WRITE_FUNC(OP_CLOSE_UPVALUE)
CREATE_WRITE_FUNC(OP_MOD)

CREATE_EXTENDED_WRITE_FUNC(OP_VAR_DEFINE, OP_GLOBAL_DEFINE_LONG)
CREATE_EXTENDED_WRITE_FUNC(OP_VAR_GET, OP_GLOBAL_GET_LONG)
CREATE_EXTENDED_WRITE_FUNC(OP_VAR_SET, OP_GLOBAL_SET_LONG)
CREATE_EXTENDED_WRITE_FUNC(OP_GLOBAL_DEFINE, OP_GLOBAL_DEFINE_LONG)
CREATE_EXTENDED_WRITE_FUNC(OP_PROPERTY_DEFINE, OP_PROPERTY_DEFINE_LONG)
CREATE_EXTENDED_WRITE_FUNC(OP_GLOBAL_GET, OP_GLOBAL_GET_LONG)
CREATE_EXTENDED_WRITE_FUNC(OP_GLOBAL_SET, OP_GLOBAL_SET_LONG)
CREATE_EXTENDED_WRITE_FUNC(OP_UPVALUE_GET, OP_UPVALUE_SET_LONG)
CREATE_EXTENDED_WRITE_FUNC(OP_UPVALUE_SET, OP_UPVALUE_SET_LONG)

static inline void OP_UPVALUE_DEFINE_WRITE(lox_fn_t *fn, uint32_t idx,
					   uint8_t flags, uint32_t line)
{
	__extended_op(&fn->chunk, OP_UPVALUE_DEFINE, OP_UPVALUE_DEFINE_LONG,
		      idx, line);
	chunk_write_code(&fn->chunk, (code_t)flags, line);
}

static inline void OP_BANG_EQ_WRITE(lox_fn_t *fn, uint32_t line)
{
	OP_EQUAL_WRITE(fn, line);
	OP_NOT_WRITE(fn, line);
}

static inline void OP_GREATER_EQ_WRITE(lox_fn_t *fn, uint32_t line)
{
	OP_LESS_WRITE(fn, line);
	OP_NOT_WRITE(fn, line);
}

static inline void OP_LESS_EQ_WRITE(lox_fn_t *fn, uint32_t line)
{
	OP_GREATER_WRITE(fn, line);
	OP_NOT_WRITE(fn, line);
}

#undef CREATE_JUMP_FUNC
#undef CREATE_EXTENDED_WRITE_FUNC
#undef CREATE_WRITE_FUNC
#undef FUNC_NAME_OF

#endif // __CLOX_COMPILER_OPS_FUNC_H__