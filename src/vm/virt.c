#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "util/common.h"
#include "virt.h"
#include "debug/debug.h"
#include "ops/ops.h"
#include "compiler/compiler.h"

#define BINARY_OP(vm, op)                                                      \
	do {                                                                   \
		lox_val_t b = __vm_pop_const(vm);                              \
		lox_val_t a = __vm_pop_const(vm);                              \
		__vm_push_const(vm, a op b);                                   \
	} while (false)

static enum vm_res __vm_run(vm_t *vm);
static void __vm_push_const(vm_t *vm, lox_val_t val);
static lox_val_t __vm_pop_const(vm_t *vm);
static void __vm_proc_const(vm_t *vm);
static void __vm_proc_const_long(vm_t *vm);
static void __vm_proc_negate_in_place(vm_t *vm);
static lox_val_t *__vm_peek_const_ptr(vm_t *vm);
static inline void __vm_assert_inst_ptr_valid(const vm_t *vm);
static inline char __vm_read_byte(vm_t *vm);

vm_t vm_init()
{
	vm_t vm = (vm_t){ NULL, NULL, list_new(lox_val_t) };
	list_reset(&vm.stack);

	return vm;
}

enum vm_res vm_interpret(vm_t *vm, const char *src)
{
	enum vm_res res;
	chunk_t chunk = chunk_new();

	if (!compile(src, &chunk)) {
		chunk_free(&chunk);
		return INTERPRET_COMPILE_ERROR;
	}

	vm->chunk = &chunk;
	vm->ip = vm->chunk->code.data;

	res = __vm_run(vm);
	chunk_free(&chunk);
	return res;
}

void vm_free(vm_t *vm)
{
	list_free(&vm->stack);
}

static enum vm_res __vm_run(vm_t *vm)
{
	while (true) {
		uint8_t instr;
#ifdef DEBUG_TRACE_EXECUTION
		printf("\t\t[");
		for (size_t idx = 0; idx < vm->stack.cnt; idx++) {
			PRINT_VAL(*((lox_val_t *)list_get(&vm->stack, idx)));
			printf(", ");
		}
		puts("]");
		disassem_inst(vm->chunk, (int)(vm->ip - vm->chunk->code.data));
#endif // DEBUG_TRACE_EXECUTION

		switch (instr = __vm_read_byte(vm)) {
		case OP_CONSTANT:
			__vm_proc_const(vm);
			break;

		case OP_CONSTANT_LONG:
			__vm_proc_const_long(vm);
			break;

		case OP_ADD:
			BINARY_OP(vm, +);
			break;

		case OP_SUBTRACT:
			BINARY_OP(vm, -);
			break;

		case OP_MULTIPLY:
			BINARY_OP(vm, *);
			break;

		case OP_DIVIDE:
			BINARY_OP(vm, /);
			break;

		case OP_NEGATE:
			__vm_proc_negate_in_place(vm);
			break;

		case OP_RETURN:
			PRINT_VAL(__vm_pop_const(vm));
			puts(" returned");
			return INTERPRET_OK;

		default:
			printf("UNKNOWN OPCODE: %d\n", instr);
			break;
		}
	}
}

static void __vm_proc_const(vm_t *vm)
{
	__vm_assert_inst_ptr_valid(vm);
	__vm_push_const(vm, chunk_get_const(vm->chunk, __vm_read_byte(vm)));
}

static void __vm_proc_const_long(vm_t *vm)
{
#define GET_CONST_LONG()                                                       \
	chunk_get_const(vm->chunk, *((uint32_t *)vm->ip) & CONST_LONG_MASK)

	__vm_assert_inst_ptr_valid(vm);
	__vm_push_const(vm, GET_CONST_LONG());
	vm->ip += CONST_LONG_SZ;

#undef GET_CONST_LONG
}

static void __vm_proc_negate_in_place(vm_t *vm)
{
#define PEEK_LIST_PTR() ((lox_val_t *)list_peek(&vm->stack))

	assert(("value stack can not be empty", vm->stack.cap));

	*(__vm_peek_const_ptr(vm)) = -*(__vm_peek_const_ptr(vm));

#undef PEEK_LIST_PTR
}

static lox_val_t *__vm_peek_const_ptr(vm_t *vm)
{
	return (lox_val_t *)list_peek(&vm->stack);
}

static lox_val_t __vm_pop_const(vm_t *vm)
{
	return *((lox_val_t *)list_pop(&vm->stack));
}

static void __vm_push_const(vm_t *vm, lox_val_t val)
{
	list_push(&vm->stack, &val);
}

static inline void __vm_assert_inst_ptr_valid(const vm_t *vm)
{
	assert(("Instruction pointer has passed code end",
		vm->ip < vm->chunk->code.data + (vm->chunk->code.cnt *
						 vm->chunk->code.type_sz)));
}

static inline char __vm_read_byte(vm_t *vm)
{
	return *vm->ip++;
}