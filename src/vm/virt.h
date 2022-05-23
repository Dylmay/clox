#ifndef __CLOX_VM_VIRT_H__
#define __CLOX_VM_VIRT_H__

#include "chunk/chunk.h"

typedef struct {
	chunk_t *chunk;
	uint8_t *ip;
	list_t stack;
} vm_t;

enum vm_res { INTERPRET_OK, INTERPRET_COMPILE_ERROR, INTERPRET_RUNTIME_ERROR };

vm_t vm_init();
enum vm_res vm_interpret(vm_t *vm, const char *src);
void vm_free(vm_t *vm);

#endif // __CLOX_VM_VIRT_H__