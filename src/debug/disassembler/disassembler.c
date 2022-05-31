#include <stdio.h>
#include <string.h>
#include "disassembler.h"
#include "ops/ops.h"
#include "val/val_func.h"

static size_t __simple_instr(const char *, size_t);
static size_t __const_instr(const char *, chunk_t *, size_t);
static size_t __const_long_instr(const char *, chunk_t *, size_t);

void disassem_chunk(chunk_t *chnk, const char *name)
{
	printf("== %s ==\n", name);
	puts("| OFFSET | LINE |        OP NAME       | VALUE |");

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

	default:
		printf("Unknown opcode %u\n", instruction);
		return offset + 1;

#undef CASE_SIMPLE_INSTR
	}
}

static size_t __simple_instr(const char *name, size_t offset)
{
	printf("%s\n", name);

	return offset + 1;
}

static size_t __const_instr(const char *name, chunk_t *chunk, size_t offset)
{
	code_t const_pos = chunk_get_code(chunk, offset + 1);
	printf(" %-20s | %5d | ", name, const_pos);
	val_print(chunk_get_const(chunk, const_pos));
	puts("");

	return offset + 2;
}

static size_t __const_long_instr(const char *name, chunk_t *chunk,
				 size_t offset)
{
#define GET_CONST_POS()                                                        \
	(*((uint32_t *)list_get(&chunk->code, offset + 1)) & CONST_LONG_MASK)

	uint32_t const_offset = GET_CONST_POS();

	printf(" %-20s | %5d | ", name, const_offset);
	val_print(chunk_get_const(chunk, const_offset));

	puts("");

	return offset + 4;
#undef GET_CONST_POS
}