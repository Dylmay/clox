#include <stdio.h>

#include "dissasembler.h"

static int simple_instr(const char *, size_t);
static int const_instr(const char *, chunk_t *, size_t);

void disassem_chunk(chunk_t *chnk, const char *name)
{
	printf("== %s ==\n", name);

	size_t offset = 0;
	while (offset < chnk->code.cnt) {
		offset = disassem_inst(chnk, offset);
	}
}

size_t disassem_inst(chunk_t *chunk, size_t offset)
{
	printf("%04lu ", offset);

	if (offset > 0 && chunk_get_line(chunk, offset) ==
				  chunk_get_line(chunk, offset - 1)) {
		printf("   | ");
	} else {
		printf("%4d ", chunk_get_line(chunk, offset));
	}

	code_t instruction = chunk_get_code(chunk, offset);

	switch (instruction) {
	case OP_RETURN:
		return simple_instr("OP_RETURN", offset);

	case OP_CONSTANT:
		return const_instr("OP_CONSTANT", chunk, offset);

	default:
		printf("Unknown opcode %u\n", instruction);
		return offset + 1;
	}
}

static int simple_instr(const char *name, size_t offset)
{
	printf("%s\n", name);

	return offset + 1;
}

static int const_instr(const char *name, chunk_t *chunk, size_t offset)
{
	code_t const_pos = chunk_get_code(chunk, offset + 1);
	printf("%-10s %4d '", name, const_pos);
	PRINT_VAL(chunk_get_const(chunk, const_pos));
	puts("'");

	return offset + 2;
}