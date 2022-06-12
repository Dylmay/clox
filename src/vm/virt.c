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
#include "util/map/hash_util.h"

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
static void __vm_assign_object(vm_t *vm, lox_obj_t *obj);
static void __vm_str_concat(vm_t *vm);
static void __vm_free_objects(vm_t *vm);
static void __vm_define_global(vm_t *vm, lox_str_t *glbl,
			       lox_val_t *val);
static lox_val_t *__vm_get_global(vm_t *vm, lox_str_t *glbl);
static bool __vm_set_global(vm_t *vm, lox_str_t *glbl, lox_val_t *val);

#define VM_PEEK_NUM(vm, dist) (__vm_peek_const_ptr(vm, dist)->as.number)
#define VM_PEEK_BOOL(vm, dist) (__vm_peek_const_ptr(vm, dist)->as.boolean)

vm_t vm_init()
{
	vm_t vm = (vm_t){
		NULL,
		chunk_new(),
		list_of_type(lox_val_t),
		map_of_type(lox_val_t, &obj_str_gen_hash),
		NULL,
	};
	list_reset(&vm.stack);

	return vm;
}

enum vm_res vm_interpret(vm_t *vm, const char *src)
{
	enum vm_res res;

	if (chunk_has_state(&vm->chunk)) {
		chunk_t prev_chunk = vm->chunk;
		vm->chunk = chunk_using_state(vm->chunk.vals);
		chunk_free(&prev_chunk, false);
	}

	if (!compile(src, &vm->chunk)) {
		return INTERPRET_COMPILE_ERROR;
	}

	vm->ip = vm->chunk.code.data;

	res = __vm_run(vm);
	return res;
}

void vm_free(vm_t *vm)
{
	list_free(&vm->stack);
	__vm_free_objects(vm);
	map_free(&vm->globals);
	chunk_free(&vm->chunk, true);
}

void _var_prnt(void *key, void *value)
{
	lox_str_t* name = (lox_str_t *)key;
	lox_val_t val = *((lox_val_t *)value);

	printf("%s = ", name->chars);
	val_print(val);
	puts("");
}

void vm_print_globals(vm_t *vm)
{
	map_entry_for_each(&vm->globals, &_var_prnt);
}

void vm_print_stack(vm_t *vm)
{
	printf("[");
  for (size_t idx = 0; idx < vm->stack.cnt; idx++) {
    val_print(*((lox_val_t *)list_get(&vm->stack, idx)));
    printf(", ");
  }
  puts("]");
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
    printf("\t\t");
		vm_print_stack(vm);
		vm_print_globals(vm);
		disassem_inst(&vm->chunk, __vm_instr_offset(vm));
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

		case OP_PRINT:
			val_print(__vm_pop_const(vm));
			puts("");
			break;

		case OP_POP:
			__vm_pop_const(vm);
			break;

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

		case OP_GLOBAL_DEFINE: {
			lox_str_t* glbl = (lox_str_t*)__vm_pop_const(vm).as.obj;
			lox_val_t *val = __vm_peek_const_ptr(vm, 0);
			__vm_define_global(vm, glbl, val);
			__vm_pop_const(vm);

		}	break;

		case OP_GLOBAL_GET: {
			lox_str_t *glbl =
				(lox_str_t *)__vm_pop_const(vm).as.obj;

			lox_val_t* val = __vm_get_global(vm, glbl);

			if (!val) {
				__vm_runtime_error(vm,
						   "Undefined variable '%s'.",
						   glbl->chars);

				return INTERPRET_RUNTIME_ERROR;
			}
			__vm_push_const(vm, *(val));
		} break;

		case OP_GLOBAL_SET: {
			lox_str_t *glbl =
				(lox_str_t *)__vm_pop_const(vm).as.obj;

			lox_val_t *val = __vm_peek_const_ptr(vm, 0);
			if (!__vm_set_global(vm, glbl, val)) {
				__vm_runtime_error(vm, "Undefined variable '%s'.",
						   glbl->chars);
				return INTERPRET_RUNTIME_ERROR;
			}
		}

		case OP_RETURN:
			return INTERPRET_OK;

		default:
			printf("UNKNOWN OPCODE: %d\n", instr);
			return INTERPRET_RUNTIME_ERROR;
		}
	}

#undef BINARY_OP
#undef NUMERICAL_OP
#undef COMPARISON_OP
}

static void __vm_define_global(vm_t *vm, lox_str_t *glbl, lox_val_t* val)
{
	map_insert(&vm->globals, glbl, val);
}

static lox_val_t *__vm_get_global(vm_t *vm, lox_str_t *glbl)
{
	return map_get(&vm->globals, glbl);
}

static bool __vm_set_global(vm_t *vm, lox_str_t *glbl, lox_val_t *val)
{
	return map_set(&vm->globals, glbl, val);
}

static void __vm_proc_const(vm_t *vm)
{
	__vm_assert_inst_ptr_valid(vm);
	__vm_push_const(vm, chunk_get_const(&vm->chunk, __vm_read_byte(vm)));
}

static void __vm_proc_const_long(vm_t *vm)
{
#define GET_CONST_LONG()                                                       \
	chunk_get_const(&vm->chunk, *((uint32_t *)vm->ip) & CONST_LONG_MASK)

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

	size_t offset = ((size_t)__vm_instr_offset(vm)) - 1;
	int line = chunk_get_line(&vm->chunk, offset);
	fprintf(stderr, "Lox Runtime Error at line %d\n", line);
	list_reset(&vm->stack);
}

static inline void __vm_assert_inst_ptr_valid(const vm_t *vm)
{
	assert(("Instruction pointer has passed code end",
		vm->ip < vm->chunk.code.data + (vm->chunk.code.cnt *
						 vm->chunk.code.type_sz)));
}

static inline char __vm_read_byte(vm_t *vm)
{
	return *vm->ip++;
}

static inline int __vm_instr_offset(const vm_t *vm)
{
	return (int)(vm->ip - vm->chunk.code.data);
}

static void __vm_str_concat(vm_t *vm)
{
	const lox_str_t *b_str = OBJECT_AS_STRING(__vm_pop_const(vm));
	const lox_str_t *a_str = OBJECT_AS_STRING(__vm_pop_const(vm));
	lox_str_t *concat_str = object_str_concat(a_str, b_str);

	__vm_assign_object(vm, (lox_obj_t*)concat_str);
	__vm_push_const(vm, VAL_CREATE_OBJ(concat_str));
}

static void __vm_assign_object(vm_t *vm, lox_obj_t *obj)
{
	obj->next = vm->objects;
	vm->objects = obj->next;
}

static void __vm_free_objects(vm_t *vm)
{
	lox_obj_t *obj = vm->objects;

	while (obj) {
		lox_obj_t *next = obj->next;
		object_free(obj);
		obj = next;
	}
	vm->objects = NULL;
}