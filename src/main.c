#include <stdio.h>

// #include "util/common.h"
#include "ops/ops.h"
#include "chunk/chunk.h"
#include "dissasembler/dissasembler.h"

int main(int argc, const char *argv[])
{
	chunk_t chunk = chunk_new();
	// op_write_const(&chunk, 1.23, 123);

	int i = 0;
	for (; i <= 500; i++) {
		op_write_const(&chunk, i % 2, i + 1);
	}
	op_write_return(&chunk, i);

	disassem_chunk(&chunk, "Test Chunk");

	chunk_free(&chunk);

	return 0;
}