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
static void __vm_define_global(vm_t *vm, lox_val_t *val);
static lox_val_t *__vm_get_global(vm_t *vm, uint32_t glbl);
static bool __vm_set_global(vm_t *vm, uint32_t glbl, lox_val_t *val);
static void __vm_proc_const(vm_t *vm, uint32_t idx);
static uint32_t __vm_proc_idx(vm_t *vm);
static uint32_t __vm_proc_idx_ext(vm_t *vm);

#define VM_PEEK_NUM(vm, dist) (__vm_peek_const_ptr(vm, dist)->as.number)
#define VM_PEEK_BOOL(vm, dist) (__vm_peek_const_ptr(vm, dist)->as.boolean)

struct var_printer {
	for_each_entry_t for_each;
	list_t* vm_globals;
};

vm_t vm_init()
{
	vm_t vm = (vm_t)
	{
		.chunk = chunk_new(),
		.globals = list_of_type(lox_val_t),
		.stack = list_of_type(lox_val_t),
		.ip = NULL,
		.objects = NULL,
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

#ifdef DEBUG_TRACE_EXECUTION
	if (res == INTERPRET_OK) {
		vm_print_globals(vm);
	}
#endif // DEBUG_TRACE_EXECUTION
	return res;
}

void vm_free(vm_t *vm)
{
	list_free(&vm->stack);
	list_free(&vm->globals);
	__vm_free_objects(vm);
	chunk_free(&vm->chunk, true);
}

void _var_prnt(struct map_entry *entry, for_each_entry_t *d)
{
	struct var_printer *data = (struct var_printer *)d;
	lox_str_t* name = (lox_str_t *)entry->key;
	uint32_t idx  = *((uint32_t *)entry->value);

	printf("\t(%u) %s: ", idx , name->chars);
	val_print(*((lox_val_t *)list_get(data->vm_globals, idx)));
	puts("");
}

void vm_print_globals(vm_t *vm)
{
	struct var_printer glbl_prnt = (struct var_printer){
		.for_each = {
			.func = &_var_prnt,
    },
		.vm_globals = &vm->globals,
	};

	puts("Globals: {");
	map_entry_for_each(&vm->chunk.vals.lookup, (for_each_entry_t *) &glbl_prnt);
	puts("}");
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
		disassem_inst(&vm->chunk, __vm_instr_offset(vm));
#endif // DEBUG_TRACE_EXECUTION

		switch (instr = __vm_read_byte(vm)) {
		case OP_CONSTANT:
			__vm_proc_const(vm, __vm_proc_idx(vm));
			break;

		case OP_CONSTANT_LONG:
			__vm_proc_const(vm, __vm_proc_idx_ext(vm));
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
			__vm_proc_idx(vm);
			lox_val_t *val = __vm_peek_const_ptr(vm, 0);

			__vm_define_global(vm, val);
			__vm_pop_const(vm);
		} break;

		case OP_GLOBAL_DEFINE_LONG: {
			__vm_proc_idx_ext(vm);
			lox_val_t *val = __vm_peek_const_ptr(vm, 0);

			__vm_define_global(vm, val);
			__vm_pop_const(vm);
		} break;

		case OP_GLOBAL_GET: {
			uint32_t idx = __vm_proc_idx(vm);
			lox_val_t* val = __vm_get_global(vm, idx);

			if (!val) {
				lox_str_t *expected = lookup_by_index(
					&vm->chunk.vals.lookup, idx);

        __vm_runtime_error(vm,"Undefined variable '%s'.",
              expected->chars);
				lookup_remove(&vm->chunk.vals.lookup, expected);

        return INTERPRET_RUNTIME_ERROR;
			}
			__vm_push_const(vm, *(val));
		} break;

    case OP_GLOBAL_GET_LONG: {
			uint32_t idx = __vm_proc_idx_ext(vm);
			lox_val_t* val = __vm_get_global(vm, idx);

			if (!val) {
				lox_str_t *expected = lookup_by_index(
					&vm->chunk.vals.lookup, idx);

        __vm_runtime_error(vm,"Undefined variable '%s'.",
              expected->chars);
				lookup_remove(&vm->chunk.vals.lookup, expected);

        return INTERPRET_RUNTIME_ERROR;
			}
			__vm_push_const(vm, *(val));
		} break;

		case OP_GLOBAL_SET: {
			uint32_t idx = __vm_proc_idx(vm);
			lox_val_t *val = __vm_peek_const_ptr(vm, 0);

			if (!__vm_set_global(vm, idx, val)) {
				__vm_runtime_error(vm, "Undefined variable '%s'.",
							lookup_by_index(&vm->chunk.vals.lookup, idx)->chars);
				lookup_remove(&vm->chunk.vals.lookup, val->as.obj);
				return INTERPRET_RUNTIME_ERROR;
			}
		} break;

		case OP_GLOBAL_SET_LONG: {
			uint32_t idx = __vm_proc_idx_ext(vm);
			lox_val_t *val = __vm_peek_const_ptr(vm, 0);

			if (!__vm_set_global(vm, idx, val)) {
				__vm_runtime_error(vm, "Undefined variable '%s'.",
							lookup_by_index(&vm->chunk.vals.lookup, idx)->chars);
				lookup_remove(&vm->chunk.vals.lookup, val->as.obj);
				return INTERPRET_RUNTIME_ERROR;
			}
		} break;

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

static void __vm_define_global(vm_t *vm, lox_val_t* val)
{
	list_push(&vm->globals, val);
}

static lox_val_t *__vm_get_global(vm_t *vm, uint32_t glbl)
{
	if (glbl >= vm->globals.cnt) {
		return NULL;
	}

	return (lox_val_t *) list_get(&vm->globals, glbl);
}

static bool __vm_set_global(vm_t *vm, uint32_t glbl, lox_val_t *val)
{
	if (glbl >= vm->globals.cnt) {
		return false;
	}

	lox_val_t *val_ptr = (lox_val_t *) list_get(&vm->globals, glbl);
	memcpy(val_ptr, val, sizeof(lox_val_t));
	return true;
}


static uint32_t __vm_proc_idx(vm_t *vm)
{
	__vm_assert_inst_ptr_valid(vm);
	return __vm_read_byte(vm);
}

static uint32_t __vm_proc_idx_ext(vm_t *vm)
{
	uint32_t idx = *((uint32_t *)vm->ip) & EXT_CODE_MASK;
	vm->ip += EXT_CODE_SZ;

	return idx;
}

static void __vm_proc_const(vm_t *vm, uint32_t idx)
{
	__vm_assert_inst_ptr_valid(vm);
	__vm_push_const(vm, chunk_get_const(&vm->chunk, idx));
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
	interner_t *str_interner = &vm->chunk.vals.strings;

	const lox_str_t *b_str = OBJECT_AS_STRING(__vm_pop_const(vm));
	const lox_str_t *a_str = OBJECT_AS_STRING(__vm_pop_const(vm));
	lox_str_t *concat_str = object_str_concat(a_str, b_str);

	lox_str_t *interned_str = intern_get_str(str_interner, concat_str);

	if (interned_str) {
		object_free((struct object *)concat_str);
	} else {
		intern_insert(str_interner, concat_str);
		interned_str = concat_str;
	}

	__vm_assign_object(vm, (lox_obj_t *)interned_str);
	__vm_push_const(vm, VAL_CREATE_OBJ(interned_str));
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