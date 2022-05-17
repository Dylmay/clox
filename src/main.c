#include <stdio.h>

// #include "util/common.h"
#include "chunk/chunk.h"
#include "util/list/list.h"
#include "dissasembler/dissasembler.h"

int main(int argc, const char *argv[])
{
	chunk_t chunk = chunk_new();
	size_t const_pos = chunk_write_const(&chunk, 1.23);
	chunk_write_code(&chunk, OP_CONSTANT, 1);
	chunk_write_code(&chunk, const_pos, 1);
	chunk_write_code(&chunk, OP_RETURN, 1);

	disassem_chunk(&chunk, "Test Chunk");

	chunk_free(&chunk);

	return 0;
}