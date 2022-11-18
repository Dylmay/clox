#include <stdio.h>
#include <string.h>
#include "disassembler.h"
#include "ops/ops_name.h"
#include "val/func/val_func.h"
#include "chunk/func/chunk_func.h"

static size_t __simple_instr(const char *, size_t);
static size_t __const_instr(const char *, chunk_t *, uint32_t);
static size_t __const_long_instr(const char *, chunk_t *, uint32_t);
static size_t __closure_instr(const char *, chunk_t *, uint32_t);
static size_t __closure_long_instr(const char *, chunk_t *, uint32_t);
static size_t __var_instr(const char *, chunk_t *, uint32_t);
static size_t __upval_def_instr(const char *, chunk_t *, uint32_t);
static size_t __upval_def_long_instr(const char *, chunk_t *, uint32_t);
static size_t __var_long_instr(const char *, chunk_t *, uint32_t);
static size_t __pop_count_instr(const char *, chunk_t *, uint32_t);
static size_t __jump_instr(const char *, chunk_t *, uint32_t);
static uint32_t __get_ext_pos(chunk_t *, uint32_t);
static size_t __call_instr(const char *, chunk_t *, size_t);
static void __print_const(const char *, lox_val_t, uint32_t);

void disassem_chunk(chunk_t *chnk, const char *name)
{
	printf("== %s ==\n", name);
	puts("| OFFSET | LINE |        OP NAME       | VALUE");

	size_t offset = 0;
	while (offset < chnk->code.cnt) {
		offset = disassem_inst(chnk, offset);
	}
}

size_t disassem_inst(chunk_t *chunk, size_t offset)
{
	printf("| %06lu |", offset);

	if (offset > 0 && chunk_get_line(chunk, offset) ==
				  chunk_get_line(chunk, offset - 1)) {
		printf(" ^^^^ |");
	} else {
		printf(" %4lu |", chunk_get_line(chunk, offset));
	}

	code_t instruction = chunk_get_code(chunk, offset);

	switch (instruction) {
#define CASE_SIMPLE_INSTR(instr)                                               \
	case instr:                                                            \
		return __simple_instr(op_name(instr), offset)
	case OP_CONSTANT:
		return __const_instr(op_name(instruction), chunk, offset);

	case OP_CONSTANT_LONG:
		return __const_long_instr(op_name(instruction), chunk, offset);

	case OP_CLOSURE:
		return __closure_instr(op_name(instruction), chunk, offset);

	case OP_CLOSURE_LONG:
		return __closure_long_instr(op_name(instruction), chunk,
					    offset);

	case OP_UPVALUE_GET:
		return __var_instr(op_name(instruction), chunk, offset);

	case OP_UPVALUE_GET_LONG:
		return __var_long_instr(op_name(instruction), chunk, offset);

	case OP_UPVALUE_SET:
		return __var_instr(op_name(instruction), chunk, offset);

	case OP_UPVALUE_SET_LONG:
		return __var_long_instr(op_name(instruction), chunk, offset);

	case OP_VAR_DEFINE:
		return __var_instr(op_name(instruction), chunk, offset);

	case OP_VAR_DEFINE_LONG:
		return __var_long_instr(op_name(instruction), chunk, offset);

	case OP_VAR_SET:
		return __var_instr(op_name(instruction), chunk, offset);

	case OP_VAR_SET_LONG:
		return __var_long_instr(op_name(instruction), chunk, offset);

	case OP_VAR_GET:
		return __var_instr(op_name(instruction), chunk, offset);

	case OP_UPVALUE_DEFINE:
		return __upval_def_instr(op_name(instruction), chunk, offset);

	case OP_UPVALUE_DEFINE_LONG:
		return __upval_def_long_instr(op_name(instruction), chunk,
					      offset);

	case OP_VAR_GET_LONG:
		return __var_long_instr(op_name(instruction), chunk, offset);

	case OP_GLOBAL_DEFINE:
		return __var_instr(op_name(instruction), chunk, offset);

	case OP_GLOBAL_DEFINE_LONG:
		return __var_long_instr(op_name(instruction), chunk, offset);

	case OP_GLOBAL_SET:
		return __var_instr(op_name(instruction), chunk, offset);

	case OP_GLOBAL_SET_LONG:
		return __var_long_instr(op_name(instruction), chunk, offset);

	case OP_GLOBAL_GET:
		return __var_instr(op_name(instruction), chunk, offset);

	case OP_GLOBAL_GET_LONG:
		return __var_long_instr(op_name(instruction), chunk, offset);

	case OP_POP_COUNT:
		return __pop_count_instr(op_name(instruction), chunk, offset);

	case OP_JUMP:
		return __jump_instr(op_name(instruction), chunk, offset);

	case OP_JUMP_IF_FALSE:
		return __jump_instr(op_name(instruction), chunk, offset);

	case OP_CALL:
		return __call_instr(op_name(instruction), chunk, offset);

		CASE_SIMPLE_INSTR(OP_RETURN);

		CASE_SIMPLE_INSTR(OP_NIL);

		CASE_SIMPLE_INSTR(OP_TRUE);

		CASE_SIMPLE_INSTR(OP_FALSE);

		CASE_SIMPLE_INSTR(OP_ADD);

		CASE_SIMPLE_INSTR(OP_SUBTRACT);

		CASE_SIMPLE_INSTR(OP_DIVIDE);

		CASE_SIMPLE_INSTR(OP_MULTIPLY);

		CASE_SIMPLE_INSTR(OP_NEGATE);

		CASE_SIMPLE_INSTR(OP_NOT);

		CASE_SIMPLE_INSTR(OP_EQUAL);

		CASE_SIMPLE_INSTR(OP_GREATER);

		CASE_SIMPLE_INSTR(OP_LESS);

		CASE_SIMPLE_INSTR(OP_POP);

		CASE_SIMPLE_INSTR(OP_MOD);

		CASE_SIMPLE_INSTR(OP_NOP);

	default:
		printf("Unknown opcode %u\n", instruction);
		return offset + 1;

#undef CASE_SIMPLE_INSTR
	}
}

static size_t __call_instr(const char *name, chunk_t *chunk, size_t offset)
{
	printf(" %-20s | %04d\n", name, chunk_get_code(chunk, offset + 1));

	return offset + 2;
}

static size_t __simple_instr(const char *name, size_t offset)
{
	printf(" %s\n", name);

	return offset + 1;
}

static size_t __const_instr(const char *name, chunk_t *chunk, uint32_t offset)
{
	code_t const_pos = chunk_get_code(chunk, offset + 1);
	__print_const(name, chunk_get_const(chunk, const_pos), const_pos);

	return offset + 2;
}

static size_t __const_long_instr(const char *name, chunk_t *chunk,
				 uint32_t offset)
{
	uint32_t const_pos = __get_ext_pos(chunk, offset);
	__print_const(name, chunk_get_const(chunk, const_pos), const_pos);

	return offset + 4;
}

static size_t __closure_instr(const char *name, chunk_t *chunk, uint32_t offset)
{
	code_t const_pos = chunk_get_code(chunk, offset + 1);
	__print_const(name, chunk_get_const(chunk, const_pos), const_pos);

	return offset + 2;
}

static size_t __closure_long_instr(const char *name, chunk_t *chunk,
				   uint32_t offset)
{
	uint32_t const_pos = __get_ext_pos(chunk, offset);
	__print_const(name, chunk_get_const(chunk, const_pos), const_pos);

	return offset + 4;
}

static size_t __var_instr(const char *name, chunk_t *chunk, uint32_t offset)
{
	code_t var_pos = chunk_get_code(chunk, offset + 1);
	printf(" %-20s |  %04d | ", name, var_pos);
	puts("");

	return offset + 2;
}

static size_t __var_long_instr(const char *name, chunk_t *chunk,
			       uint32_t offset)
{
	uint32_t var_pos = __get_ext_pos(chunk, offset);
	printf(" %-20s |  %04d | ", name, var_pos);
	puts("");

	return offset + 2;
}

static size_t __pop_count_instr(const char *name, chunk_t *chunk,
				uint32_t offset)
{
	code_t pop_cnt = chunk_get_code(chunk, offset + 1);
	printf(" %-20s |  %04d", name, pop_cnt);
	puts("");

	return offset + 2;
}

static size_t __jump_instr(const char *name, chunk_t *chunk, uint32_t offset)
{
	int16_t jump_pos = *((int16_t *)list_get(&chunk->code, offset + 1));

	printf(" %-20s | *%04d -> *%04d ", name, offset, offset + 3 + jump_pos);
	puts("");

	return offset + 3;
}

static uint32_t __get_ext_pos(chunk_t *chunk, uint32_t offset)
{
	return *((uint32_t *)list_get(&chunk->code, offset + 1)) &
	       EXT_CODE_MASK;
}

static void __print_const(const char *name, lox_val_t const_val,
			  uint32_t const_pos)
{
	printf(" %-20s | @%04d $ '", name, const_pos);
	val_print(const_val);
	putchar('\'');
	puts("");
}

static size_t __upval_def_instr(const char *name, chunk_t *chunk,
				uint32_t offset)
{
	return __var_instr(name, chunk, offset) + 1;
}

static size_t __upval_def_long_instr(const char *name, chunk_t *chunk,
				     uint32_t offset)
{
	return __var_long_instr(name, chunk, offset) + 1;
}
