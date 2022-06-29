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
#include "util/string/string_util.h"
#include "util/list/list_iterator.h"

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
static void __vm_define_var(vm_t *vm, lox_val_t *val);
static lox_val_t *__vm_get_var(vm_t *vm, uint32_t glbl);
static bool __vm_set_var(vm_t *vm, uint32_t glbl, lox_val_t *val);
static void __vm_proc_const(vm_t *vm, uint32_t idx);
static uint32_t __vm_proc_idx(vm_t *vm);
static uint32_t __vm_proc_idx_ext(vm_t *vm);
static int16_t __vm_proc_jump_offset(vm_t *vm);
static void __vm_discard(vm_t *vm, uint32_t discard_cnt);

#define VM_PEEK_NUM(vm, dist) (__vm_peek_const_ptr(vm, dist)->as.number)
#define VM_PEEK_BOOL(vm, dist) (__vm_peek_const_ptr(vm, dist)->as.boolean)

struct var_printer {
	for_each_entry_t for_each;
	list_t *vm_vars;
	size_t depth;
};

vm_t vm_init()
{
	vm_t vm = (vm_t){
		.chunk = chunk_new(),
		.vars = list_of_type(lox_val_t),
		.stack = list_of_type(lox_val_t),
		.ip = NULL,
		.objects = linked_list_of_type(lox_obj_t *),
	};
	list_reset(&vm.stack);

	return vm;
}

enum vm_res vm_interpret(vm_t *vm, const char *src)
{
	enum vm_res res;

	chunk_t prev_chunk = vm->chunk;
	vm->chunk = chunk_using_state(vm->chunk.vals);
	chunk_free(&prev_chunk, false);

	if (!compile(src, &vm->chunk)) {
		return INTERPRET_COMPILE_ERROR;
	}

	vm->ip = vm->chunk.code.data;

	res = __vm_run(vm);

#ifdef DEBUG_TRACE_EXECUTION
	if (res == INTERPRET_OK) {
		vm_print_vars(vm);
	}
#endif // DEBUG_TRACE_EXECUTION
	return res;
}

void vm_free(vm_t *vm)
{
	list_free(&vm->stack);
	list_free(&vm->vars);
	__vm_free_objects(vm);
	chunk_free(&vm->chunk, true);
}
void _var_prnt(map_entry_t entry, for_each_entry_t *d)
{
	struct var_printer *data = (struct var_printer *)d;
	struct string *name = (struct string *)entry.key;
	uint32_t idx = *((uint32_t *)entry.value);

	for (size_t i = 0; i < data->depth; i++) {
		putchar('\t');
	}
	printf("(%u) %s: ", idx, string_get_cstring(name));
	val_print(*((lox_val_t *)list_get(data->vm_vars, idx)));
	puts("");
}

void vm_print_vars(vm_t *vm)
{
#define INDENT_BY(i)                                                           \
	do {                                                                   \
		for (int __i = 0; __i < i; __i++) {                            \
			putchar('\t');                                         \
		}                                                              \
	} while (false)

	struct var_printer var_prnt = (struct var_printer){
		.for_each = {
			.func = &_var_prnt,
	},
		.vm_vars = &vm->vars,
		.depth = 1,
	};

	puts("Globals: {");
	map_entries_for_each(lookup_scope_at_depth(&vm->chunk.vals.lookup, 1),
			     (for_each_entry_t *)&var_prnt);
	puts("}");

	for (int depth = 2; depth < lookup_cur_depth(&vm->chunk.vals.lookup);
	     depth++) {
		var_prnt.depth = depth - 1;

		INDENT_BY(depth - 2);
		printf("Scope @%d: {", depth);
		INDENT_BY(depth - 2);
		map_entries_for_each(
			lookup_scope_at_depth(&vm->chunk.vals.lookup, depth),
			(for_each_entry_t *)&var_prnt);
		INDENT_BY(depth - 2);
		puts("}");
	}
#undef INDENT_BY
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

		case OP_POP_COUNT:
			__vm_discard(vm, __vm_proc_idx(vm));
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

		case OP_VAR_DEFINE: {
			__vm_proc_idx(vm);
			lox_val_t *val = __vm_peek_const_ptr(vm, 0);

			__vm_define_var(vm, val);
			__vm_pop_const(vm);
		} break;

		case OP_VAR_DEFINE_LONG: {
			__vm_proc_idx_ext(vm);
			lox_val_t *val = __vm_peek_const_ptr(vm, 0);

			__vm_define_var(vm, val);
			__vm_pop_const(vm);
		} break;

		case OP_VAR_GET: {
			uint32_t idx = __vm_proc_idx(vm);
			lox_val_t *val = __vm_get_var(vm, idx);

			if (!val) {
				__vm_runtime_error(vm, "Undefined variable.");
				return INTERPRET_RUNTIME_ERROR;
			}
			__vm_push_const(vm, *val);
		} break;

		case OP_VAR_GET_LONG: {
			uint32_t idx = __vm_proc_idx_ext(vm);
			lox_val_t *val = __vm_get_var(vm, idx);

			if (!val) {
				__vm_runtime_error(vm, "Undefined variable.");
				return INTERPRET_RUNTIME_ERROR;
			}
			__vm_push_const(vm, *val);
		} break;

		case OP_VAR_SET: {
			uint32_t idx = __vm_proc_idx(vm);
			lox_val_t *val = __vm_peek_const_ptr(vm, 0);

			if (!__vm_set_var(vm, idx, val)) {
				__vm_runtime_error(vm, "Undefined variable.");
				return INTERPRET_RUNTIME_ERROR;
			}
		} break;

		case OP_VAR_SET_LONG: {
			uint32_t idx = __vm_proc_idx_ext(vm);
			lox_val_t *val = __vm_peek_const_ptr(vm, 0);

			if (!__vm_set_var(vm, idx, val)) {
				__vm_runtime_error(vm, "Undefined variable.");
				return INTERPRET_RUNTIME_ERROR;
			}
		} break;

		case OP_JUMP: {
			int16_t offset = __vm_proc_jump_offset(vm);
			vm->ip += offset;
		} break;

		case OP_JUMP_IF_FALSE: {
			int16_t offset = __vm_proc_jump_offset(vm);

			if (val_is_falsey(__vm_peek_const(vm, 0))) {
				vm->ip += offset;
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

static void __vm_define_var(vm_t *vm, lox_val_t *val)
{
	list_push(&vm->vars, val);
}

static lox_val_t *__vm_get_var(vm_t *vm, uint32_t glbl)
{
	if (glbl >= vm->vars.cnt) {
		return NULL;
	}

	return (lox_val_t *)list_get(&vm->vars, glbl);
}

static bool __vm_set_var(vm_t *vm, uint32_t glbl, lox_val_t *val)
{
	if (glbl >= vm->vars.cnt) {
		return false;
	}

	lox_val_t *val_ptr = (lox_val_t *)list_get(&vm->vars, glbl);
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

static int16_t __vm_proc_jump_offset(vm_t *vm)
{
	int16_t idx = *((int16_t *)vm->ip);

	vm->ip += 2;

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
	linked_list_insert_back(&vm->objects, obj);
}

static void __vm_free_objects(vm_t *vm)
{
	list_iterator_t iter = list_iter(vm->objects, true);

	while (list_iter_has_next(&iter)) {
		lox_obj_t *next = list_iter_next(&iter);
		object_free(next);
	}
	linked_list_free(vm->objects);
}

static void __vm_discard(vm_t *vm, uint32_t discard_cnt)
{
	list_adjust_cnt(&vm->vars, list_size(&vm->vars) - discard_cnt);
}