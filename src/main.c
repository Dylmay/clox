#include <stdio.h>

#include "util/common.h"
#include "chunk/chunk.h"
#include "ops/ops.h"
#include "vm/virt.h"
// #include "debug/debug.h"

int main(int argc, const char *argv[])
{
	chunk_t chunk = chunk_new();
	vm_t virt = vm_init();
	int i = 0;
	for (; i <= 500; i++) {
		op_write_const(&chunk, i * 2, i + 1);
	}
	op_write_return(&chunk, i);

	vm_interpret(&virt, &chunk);
	chunk_free(&chunk);
	vm_free(&virt);

	return 0;
}
