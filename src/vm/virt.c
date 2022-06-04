#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "util/common.h"
#include "virt.h"
#include "debug/debug.h"
#include "ops/ops.h"
#include "val/func/val_func.h"
#include "val/func/object_func.h"
#include "compiler/compiler.h"

static enum vm_res __vm_run(vm_t *vm);
static void __vm_push_const(vm_t *vm, lox_val_t val);
static lox_val_t __vm_pop_const(vm_t *vm);
static void __vm_proc_const(vm_t *vm);
static void __vm_proc_const_long(vm_t *vm);
static void __vm_proc_negate_in_place(vm_t *vm);
static lox_val_t __vm_peek_const(vm_t *vm, size_t dist);
static lox_val_t *__vm_peek_const_ptr(vm_t *vm, size_t dist);
static inline void __vm_assert_inst_ptr_valid(const vm_t *vm);
static inline char __vm_read_byte(vm_t *vm);
static inline int __vm_instr_offset(const vm_t *vm);
static void __vm_runtime_error(vm_t *vm, const char *fmt, ...);
static void __vm_assign_object(vm_t *vm, struct object *obj);
static void __vm_str_concat(vm_t *vm);
static void __vm_free_objects(vm_t *vm);

#define VM_PEEK_NUM(vm, dist) (__vm_peek_const_ptr(vm, dist)->as.number)
#define VM_PEEK_BOOL(vm, dist) (__vm_peek_const_ptr(vm, dist)->as.boolean)

vm_t vm_init()
{
	vm_t vm = (vm_t){
		NULL,
		NULL,
		list_new(lox_val_t),
		NULL,
	};
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
	__vm_free_objects(vm);
}

static enum vm_res __vm_run(vm_t *vm)
{
#define BINARY_OP(vm, val_type, op)                                            \
	do {                                                                   \
		if (!VAL_IS_NUMBER(*(__vm_peek_const_ptr(vm, 1)))) {           \
			puts("top Not number");                                \
			printf("type %d", __vm_peek_const_ptr(vm, 1)->type);   \
		}                                                              \
		if (!VAL_IS_NUMBER(*(__vm_peek_const_ptr(vm, 0))) ||           \
		    !VAL_IS_NUMBER(*(__vm_peek_const_ptr(vm, 1)))) {           \
			__vm_runtime_error(vm, "Operands must be numbers.");   \
			return INTERPRET_RUNTIME_ERROR;                        \
		}                                                              \
		lox_num_t b = __vm_pop_const(vm).as.number;                    \
		lox_num_t a = __vm_pop_const(vm).as.number;                    \
		__vm_push_const(vm, val_type(a op b));                         \
	} while (false)

#define NUMERICAL_OP(vm, op) BINARY_OP(vm, VAL_CREATE_NUMBER, op)
#define COMPARISON_OP(vm, op) BINARY_OP(vm, VAL_CREATE_BOOL, op)

	while (true) {
		uint8_t instr;
#ifdef DEBUG_TRACE_EXECUTION
		printf("\t\t[");
		for (size_t idx = 0; idx < vm->stack.cnt; idx++) {
			val_print(*((lox_val_t *)list_get(&vm->stack, idx)));
			printf(", ");
		}
		puts("]");
		disassem_inst(vm->chunk, __vm_instr_offset(vm));
#endif // DEBUG_TRACE_EXECUTION

		switch (instr = __vm_read_byte(vm)) {
		case OP_CONSTANT:
			__vm_proc_const(vm);
			break;

		case OP_CONSTANT_LONG:
			__vm_proc_const_long(vm);
			break;

		case OP_NIL:
			__vm_push_const(vm, VAL_CREATE_NIL);
			break;

		case OP_TRUE:
			__vm_push_const(vm, VAL_CREATE_BOOL(true));
			break;

		case OP_FALSE:
			__vm_push_const(vm, VAL_CREATE_BOOL(false));
			break;

		case OP_ADD:
			if (OBJECT_IS_STRING(__vm_peek_const(vm, 0)) &&
			    OBJECT_IS_STRING(__vm_peek_const(vm, 1))) {
				__vm_str_concat(vm);
			} else if (VAL_IS_NUMBER(__vm_peek_const(vm, 0)) &&
				   VAL_IS_NUMBER(__vm_peek_const(vm, 1))) {
				NUMERICAL_OP(vm, +);
			} else {
				__vm_runtime_error(
					vm,
					"Operands must be two numbers or two strings.");
				return INTERPRET_RUNTIME_ERROR;
			}
			break;

		case OP_SUBTRACT:
			NUMERICAL_OP(vm, -);
			break;

		case OP_MULTIPLY:
			NUMERICAL_OP(vm, *);
			break;

		case OP_DIVIDE:
			NUMERICAL_OP(vm, /);
			break;

		case OP_GREATER:
			COMPARISON_OP(vm, >);
			break;

		case OP_LESS:
			COMPARISON_OP(vm, <);

		case OP_EQUAL: {
			lox_val_t b = __vm_pop_const(vm);
			lox_val_t a = __vm_pop_const(vm);

			__vm_push_const(vm, VAL_CREATE_BOOL(val_equals(a, b)));
		} break;

		case OP_NEGATE:
			if (!VAL_IS_NUMBER(__vm_peek_const(vm, 0))) {
				__vm_runtime_error(vm,
						   "Operand must be a number");
				return INTERPRET_RUNTIME_ERROR;
			}
			__vm_proc_negate_in_place(vm);
			break;

		case OP_NOT:
			__vm_push_const(vm, VAL_CREATE_BOOL(val_is_falsey(
						    __vm_pop_const(vm))));
			break;

		case OP_RETURN:
			val_print(__vm_pop_const(vm));
			puts(" returned");
			return INTERPRET_OK;

		default:
			printf("UNKNOWN OPCODE: %d\n", instr);
			break;
		}
	}

#undef BINARY_OP
#undef NUMERICAL_OP
#undef COMPARISON_OP
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
	assert(("value stack can not be empty", vm->stack.cap));

	VM_PEEK_NUM(vm, 0) = -VM_PEEK_NUM(vm, 0);
}

static lox_val_t __vm_peek_const(vm_t *vm, size_t dist)
{
	return *(__vm_peek_const_ptr(vm, dist));
}

static lox_val_t *__vm_peek_const_ptr(vm_t *vm, size_t dist)
{
	return (lox_val_t *)(list_peek_offset(&vm->stack, dist));
}

static lox_val_t __vm_pop_const(vm_t *vm)
{
	assert(("Stack cannot be empty on pop", vm->stack.cnt));

	return *((lox_val_t *)list_pop(&vm->stack));
}

static void __vm_push_const(vm_t *vm, lox_val_t val)
{
	list_push(&vm->stack, &val);
}

static void __vm_runtime_error(vm_t *vm, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	vfprintf(stderr, fmt, args);

	va_end(args);
	fputs("\n", stderr);

	size_t offset = __vm_instr_offset(vm) - 1;
	int line = chunk_get_line(vm->chunk, offset);
	fprintf(stderr, "Lox Runtime Error at line %d\n", line);
	list_reset(&vm->stack);
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

static inline int __vm_instr_offset(const vm_t *vm)
{
	return (int)(vm->ip - vm->chunk->code.data);
}

static void __vm_str_concat(vm_t *vm)
{
	const struct object_str *b_str = OBJECT_AS_STRING(__vm_pop_const(vm));
	const struct object_str *a_str = OBJECT_AS_STRING(__vm_pop_const(vm));
	const struct object_str *concat_str = object_str_concat(a_str, b_str);

	__vm_assign_object(vm, (struct object *)concat_str);
	__vm_push_const(vm, VAL_CREATE_OBJ(concat_str));
}

static void __vm_assign_object(vm_t *vm, struct object *obj)
{
	obj->next = vm->objects;
	vm->objects = obj->next;
}

static void __vm_free_objects(vm_t *vm)
{
	struct object *obj = vm->objects;

	while (obj) {
		struct object *next = obj->next;

		switch (obj->type) {
		case OBJ_STRING: {
			struct object_str *str = (struct object_str *)obj;
			FREE_ARRAY(char, str->chars, str->len + 1);
			FREE(struct object_str, str);
		} break;

		default:
			break;
		}

		obj = next;
	}
}