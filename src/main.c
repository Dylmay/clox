#include <stdio.h>

#include "util/common.h"
#include "chunk/chunk.h"
#include "ops/ops.h"
#include "vm/virt.h"
#include "debug/debug.h"
int main(int argc, const char *argv[])
{
	chunk_t chunk = chunk_new();
	vm_t virt = vm_init();

	op_write_const(&chunk, 2.5, 1);
	op_write_negate(&chunk, 1);
	op_write_const(&chunk, 5, 1);
	op_write_add(&chunk, 1);
	op_write_const(&chunk, 5, 1);
	op_write_div(&chunk, 1);
	op_write_const(&chunk, 10, 1);
	op_write_mult(&chunk, 1);
	op_write_const(&chunk, 5, 1);
	op_write_sub(&chunk, 1);
	op_write_return(&chunk, 1);

	vm_interpret(&virt, &chunk);

	chunk_free(&chunk);
	vm_free(&virt);
	return 0;
}
