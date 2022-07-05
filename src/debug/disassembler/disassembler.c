#include <stdio.h>
#include <string.h>
#include "disassembler.h"
#include "ops/ops.h"
#include "val/func/val_func.h"

static size_t __simple_instr(const char *, size_t);
static size_t __const_instr(const char *, chunk_t *, uint32_t);
static size_t __const_long_instr(const char *, chunk_t *, uint32_t);
static size_t __var_instr(const char *, chunk_t *, uint32_t);
static size_t __var_long_instr(const char *, chunk_t *, uint32_t);
static size_t __pop_count_instr(const char *, chunk_t *, uint32_t);
static uint32_t __jump_instr(const char *, chunk_t *, uint32_t);
static uint32_t __get_ext_pos(chunk_t *, uint32_t);

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
		printf(" %4d |", chunk_get_line(chunk, offset));
	}

	code_t instruction = chunk_get_code(chunk, offset);

	switch (instruction) {
#define CASE_SIMPLE_INSTR(instr)                                               \
	case instr:                                                            \
		return __simple_instr(#instr, offset)

	case OP_CONSTANT:
		return __const_instr("OP_CONSTANT", chunk, offset);

	case OP_CONSTANT_LONG:
		return __const_long_instr("OP_CONSTANT_LONG", chunk, offset);

	case OP_VAR_DEFINE:
		return __var_instr("OP_VAR_DEFINE", chunk, offset);

	case OP_VAR_DEFINE_LONG:
		return __var_long_instr("OP_VAR_DEFINE", chunk, offset);

	case OP_VAR_SET:
		return __var_instr("OP_VAR_SET", chunk, offset);

	case OP_VAR_SET_LONG:
		return __var_long_instr("OP_VAR_SET_LONG", chunk, offset);

	case OP_VAR_GET:
		return __var_instr("OP_VAR_GET", chunk, offset);

	case OP_VAR_GET_LONG:
		return __var_long_instr("OP_VAR_GET_LONG", chunk, offset);

	case OP_POP_COUNT:
		return __pop_count_instr("OP_POP_COUNT", chunk, offset);

	case OP_JUMP:
		return __jump_instr("OP_JUMP", chunk, offset);

	case OP_JUMP_IF_FALSE:
		return __jump_instr("OP_JUMP_IF_FALSE", chunk, offset);

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

		CASE_SIMPLE_INSTR(OP_PRINT);

		CASE_SIMPLE_INSTR(OP_POP);

		CASE_SIMPLE_INSTR(OP_MOD);

	default:
		printf("Unknown opcode %u\n", instruction);
		return offset + 1;

#undef CASE_SIMPLE_INSTR
	}
}

static size_t __simple_instr(const char *name, size_t offset)
{
	printf(" %s\n", name);

	return offset + 1;
}

static size_t __const_instr(const char *name, chunk_t *chunk, uint32_t offset)
{
	code_t const_pos = chunk_get_code(chunk, offset + 1);
	printf(" %-20s | @%04d $'", name, const_pos);
	val_print(chunk_get_const(chunk, const_pos));
	putchar('\'');
	puts("");

	return offset + 2;
}

static size_t __const_long_instr(const char *name, chunk_t *chunk,
				 uint32_t offset)
{
#define GET_CONST_POS()                                                        \
	(*((uint32_t *)list_get(&chunk->code, offset + 1)) & EXT_CODE_MASK)

	uint32_t const_offset = __get_ext_pos(chunk, offset);

	printf(" %-20s | @%04d $ ", name, const_offset);
	val_print(chunk_get_const(chunk, const_offset));

	puts("");

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

static uint32_t __jump_instr(const char *name, chunk_t *chunk, uint32_t offset)
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
