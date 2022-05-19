#include <stdio.h>
#include <string.h>
#include "disassembler.h"
#include "ops/ops.h"

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
	case OP_RETURN:
		return __simple_instr("OP_RETURN", offset);

	case OP_CONSTANT:
		return __const_instr("OP_CONSTANT", chunk, offset);

	case OP_CONSTANT_LONG:
		return __const_long_instr("OP_CONSTANT_LONG", chunk, offset);

	case OP_ADD:
		return __simple_instr("OP_ADD", offset);

	case OP_SUBTRACT:
		return __simple_instr("OP_SUBTRACT", offset);

	case OP_DIVIDE:
		return __simple_instr("OP_DIVIDE", offset);

	case OP_MULTIPLY:
		return __simple_instr("OP_MULTIPLY", offset);

	case OP_NEGATE:
		return __simple_instr("OP_NEGATE", offset);

	default:
		printf("Unknown opcode %u\n", instruction);
		return offset + 1;
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
	PRINT_VAL(chunk_get_const(chunk, const_pos));
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
	PRINT_VAL(chunk_get_const(chunk, const_offset));

	puts("");

	return offset + 4;
#undef GET_CONST_LONG
}