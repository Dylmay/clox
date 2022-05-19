#include <stdio.h>
#include <string.h>

#include "util/common.h"
#include "virt.h"
#include "ops/ops.h"
#include "debug/debug.h"

#define READ_BYTE() (*vm->ip++)

static vm_res_t __vm_run(vm_t *vm);
static void __vm_push_const(vm_t *vm, lox_val_t val);
static lox_val_t __vm_pop_const(vm_t *vm);
static void __vm_proc_const(vm_t *vm);
static void __vm_proc_const_long(vm_t *vm);

vm_t vm_init()
{
	vm_t vm = (vm_t){ NULL, NULL, list_new(lox_val_t) };
	list_reset(&vm.stack);

	return vm;
}

vm_res_t vm_interpret(vm_t *vm, chunk_t *chunk)
{
	vm->chunk = chunk;
	vm->ip = vm->chunk->code.data;
	return __vm_run(vm);
}

void vm_free(vm_t *vm)
{
	list_free(&vm->stack);
}

static vm_res_t __vm_run(vm_t *vm)
{
	while (true) {
		uint8_t instr;
#ifdef DEBUG_TRACE_EXECUTION
		printf("		[");
		for (size_t idx = 0; idx < vm->stack.cnt; idx++) {
			PRINT_VAL(*((lox_val_t *)list_get(&vm->stack, idx)));
			printf(", ");
		}
		puts("]");
		disassem_inst(vm->chunk, (int)(vm->ip - vm->chunk->code.data));
#endif // DEBUG_TRACE_EXECUTION

		switch (instr = READ_BYTE()) {
		case OP_CONSTANT:
			__vm_proc_const(vm);
			break;

		case OP_CONSTANT_LONG:
			__vm_proc_const_long(vm);
			break;

		case OP_RETURN:
			PRINT_VAL(__vm_pop_const(vm));
			puts(" returned");
			return INTERPRET_OK;
		}
	}
}

static void __vm_proc_const(vm_t *vm)
{
	__vm_push_const(vm, chunk_get_const(vm->chunk, READ_BYTE()));
}

static void __vm_proc_const_long(vm_t *vm)
{
#define GET_CONST_LONG()                                                       \
	chunk_get_const(vm->chunk, *((uint32_t *)vm->ip) & CONST_LONG_MASK)

	__vm_push_const(vm, GET_CONST_LONG());
	vm->ip += CONST_LONG_SZ;

#undef GET_CONST_LONG
}

static lox_val_t __vm_pop_const(vm_t *vm)
{
	return *((lox_val_t *)list_pop(&vm->stack));
}

static void __vm_push_const(vm_t *vm, lox_val_t val)
{
	list_push(&vm->stack, &val);
}

#undef READ_BYTE