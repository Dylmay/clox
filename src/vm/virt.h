#ifndef __CLOX_VM_VIRT_H__
#define __CLOX_VM_VIRT_H__

#include "chunk/chunk.h"

typedef struct {
	chunk_t *chunk;
	uint8_t *ip;
	list_t stack;
} vm_t;

typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
} vm_res_t;

vm_t vm_init();
vm_res_t vm_interpret(vm_t *vm, chunk_t *chunk);
void vm_free(vm_t *vm);

#endif // __CLOX_VM_VIRT_H__