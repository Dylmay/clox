#ifndef __CLOX_VM_VIRT_H__
#define __CLOX_VM_VIRT_H__

#include "chunk/chunk.h"
#include "val/object.h"
#include "util/list/list.h"
#include "util/list/linked_list.h"

typedef struct {
	uint8_t *ip;
	chunk_t chunk;
	list_t stack;
	list_t vars;
	linked_list_t objects;
} vm_t;

enum vm_res { INTERPRET_OK, INTERPRET_COMPILE_ERROR, INTERPRET_RUNTIME_ERROR };

vm_t vm_init();
enum vm_res vm_interpret(vm_t *vm, const char *src);
void vm_free(vm_t *vm);
void vm_print_vars(vm_t *vm);
void vm_print_stack(vm_t *vm);

#endif // __CLOX_VM_VIRT_H__