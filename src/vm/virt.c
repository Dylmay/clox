#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>

#include "util/common.h"
#include "virt.h"
#include "ops/ops.h"
#include "chunk/func/chunk_func.h"
#include "val/func/val_func.h"
#include "val/func/object_func.h"
#include "compiler/compiler.h"
#include "util/map/hash_util.h"
#include "util/string/string_util.h"

#if defined(DEBUG_PRINT_CODE) | defined(DEBUG_BENCH)
#include "debug/debug.h"
#endif

#ifdef DEBUG_BENCH
#include "ops/ops_name.h"
void _vm_print_time(struct map_entry entry, struct map_for_each_entry *_)
{
	const char *name = entry.key;
	const struct timespec *avg_time = entry.value;

	printf("  %s: ", name);
	timespec_print(*avg_time, true);
	puts("");
}
#endif

static enum vm_res __vm_run(vm_t *vm);
static void __vm_push_const(vm_t *vm, lox_val_t val);
static lox_val_t __vm_pop_const(vm_t *vm);
static void __vm_proc_negate_in_place(vm_t *vm);
static lox_val_t __vm_peek_const(vm_t *vm, size_t dist);
static lox_val_t *__vm_peek_const_ptr(vm_t *vm, size_t dist);
static void __vm_runtime_error(vm_t *vm, const char *fmt, ...);
static void __vm_str_concat(vm_t *vm);
static void __vm_define_global(vm_t *vm, lox_val_t *val, size_t idx);
static lox_val_t *__vm_get_global(vm_t *vm, uint32_t glbl);
static bool __vm_set_global(vm_t *vm, uint32_t glbl, lox_val_t *val);
static void __vm_define_var(vm_t *vm, lox_val_t *val);
static lox_val_t *__vm_get_var(vm_t *vm, uint32_t glbl);
static bool __vm_set_var(vm_t *vm, uint32_t glbl, lox_val_t *val);
static void __vm_set_main(vm_t *vm, lox_fn_t *main);
static bool __vm_call_val(vm_t *vm, lox_val_t callee, uint8_t arity);
static bool __vm_call(vm_t *vm, lox_closure_t *closure);

static void __vm_proc_const(vm_t *vm, struct vm_call_frame *frame,
			    uint32_t idx);
static inline void __frame_assert_inst_ptr_valid(const struct vm_call_frame *);
static inline char __frame_read_byte(struct vm_call_frame *);
static inline int __frame_instr_offset(const struct vm_call_frame *);
static uint32_t __frame_proc_idx(struct vm_call_frame *);
static uint32_t __frame_proc_idx_ext(struct vm_call_frame *);
static int16_t __frame_proc_jump_offset(struct vm_call_frame *);
static void __vm_discard(vm_t *vm, uint32_t discard_cnt);
static lox_upval_t *__vm_capture_upval(vm_t *vm, size_t idx);

#define VM_PEEK_NUM(vm, dist) (__vm_peek_const_ptr(vm, dist)->as.number)
#define VM_PEEK_BOOL(vm, dist) (__vm_peek_const_ptr(vm, dist)->as.boolean)

struct var_printer {
	struct map_for_each_entry for_each;
	list_t *vm_vars;
	size_t depth;
};

vm_t vm_init()
{
	vm_t vm = (vm_t){
		.stack = list_of_type(lox_val_t),
		.globals = list_of_type(lox_val_t),
		.frames = list_of_type(struct vm_call_frame),
		.state = state_new(),
#ifdef DEBUG_BENCH
		.timings_map =
			map_of_type(struct timespec, (hash_fn)&asciiz_gen_hash),
#endif
	};

	return vm;
}

enum vm_res vm_interpret(vm_t *vm, const char *src)
{
	enum vm_res res;

	lox_fn_t *fn = compile(src, &vm->state);

	if (!fn) {
		return INTERPRET_COMPILE_ERROR;
	}

	__vm_set_main(vm, fn);

#ifdef DEBUG_BENCH
	struct timespec timer;
	timer_start(&timer);
#endif

	res = __vm_run(vm);

#ifdef DEBUG_BENCH
	printf("Time taken to execute: ");
	timespec_print(timer_end(timer), true);
	puts("");

	puts("Time taken per op: {");
	struct map_for_each_entry for_each = {
		.func = &_vm_print_time,
	};
	map_entries_for_each(&vm->timings_map, &for_each);
	map_free(&vm->timings_map);
	puts("}");
#endif

	//object_free((struct object *)vm->fn);

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
	list_free(&vm->globals);
	state_free(&vm->state);
#ifdef DEBUG_BENCH
	map_free(&vm->timings_map);
#endif
}

void _var_prnt(struct map_entry entry, struct map_for_each_entry *d)
{
	struct var_printer *data = (struct var_printer *)d;
	struct string *name = (struct string *)entry.key;
	lookup_var_t var_def = *((lookup_var_t *)entry.value);

	for (size_t i = 0; i < data->depth; i++) {
		putchar('\t');
	}

	if (lookup_var_is_defined(var_def)) {
		printf("(%u) %s: ", var_def.idx, string_get_cstring(name));
		val_print(*((lox_val_t *)list_get(data->vm_vars, var_def.idx)));
	} else {
		printf("(?) %s: ", string_get_cstring(name));
	}

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
		.vm_vars = &vm->globals,
		.depth = 1,
	};

	puts("Globals: {");
	map_entries_for_each(&vm->state.globals.table,
			     (struct map_for_each_entry *)&var_prnt);
	puts("}");
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
		if (!VAL_IS_NUMBER(*(__vm_peek_const_ptr(vm, 0))) ||           \
		    !VAL_IS_NUMBER(*(__vm_peek_const_ptr(vm, 1)))) {           \
			__vm_runtime_error(vm, "Operand types must match");    \
			return INTERPRET_RUNTIME_ERROR;                        \
		}                                                              \
		lox_num_t b = __vm_pop_const(vm).as.number;                    \
		lox_num_t a = __vm_pop_const(vm).as.number;                    \
		__vm_push_const(vm, val_type(a op b));                         \
	} while (false)

#define NUMERICAL_OP(vm, op) BINARY_OP(vm, VAL_CREATE_NUMBER, op)
#define COMPARISON_OP(vm, op) BINARY_OP(vm, VAL_CREATE_BOOL, op)

	struct vm_call_frame *cur_frame = list_peek(&vm->frames);

	while (true) {
		uint8_t instr;
#ifdef DEBUG_TRACE_EXECUTION
		printf("\t\t");
		vm_print_stack(vm);
		disassem_inst(&cur_frame->closure->fn->chunk,
			      __frame_instr_offset(cur_frame));
#endif // DEBUG_TRACE_EXECUTION
#ifdef DEBUG_BENCH
		struct timespec timer;
		timer_start(&timer);
#endif

		switch (instr = __frame_read_byte(cur_frame)) {
		case OP_NOP:
			break;

		case OP_CONSTANT:
			__vm_proc_const(vm, cur_frame,
					__frame_proc_idx(cur_frame));
			break;

		case OP_CONSTANT_LONG:
			__vm_proc_const(vm, cur_frame,
					__frame_proc_idx_ext(cur_frame));
			break;

		case OP_CLOSURE: {
			lox_val_t fn =
				chunk_get_const(&cur_frame->closure->fn->chunk,
						__frame_proc_idx(cur_frame));

			assert(("closure object is not a function",
				OBJECT_IS_FN(fn)));

			lox_closure_t *closure =
				object_closure_new(OBJECT_AS_FN(fn));

			__vm_push_const(vm, VAL_CREATE_OBJ(closure));
			for (size_t i = 0; i < closure->fn->upval_cnt; i++) {
				lox_upval_t *upval;
				op_code_t opcode = __frame_read_byte(cur_frame);
				assert(("expected upval define indicator",
					opcode == OP_UPVALUE_DEFINE ||
						opcode ==
							OP_UPVALUE_DEFINE_LONG));

				uint32_t idx = __frame_proc_idx(
					cur_frame); // TODO: add support for wide commands
				bool is_local = __frame_read_byte(cur_frame);

				if (is_local) {
					upval = __vm_capture_upval(
						vm, cur_frame->stack_snapshot +
							    idx);
				} else {
					upval = object_closure_get_upval(
						cur_frame->closure, idx);
				}
				object_closure_push_upval(closure, upval);
			}
		} break;

		case OP_CLOSURE_LONG: {
			lox_val_t fn = chunk_get_const(
				&cur_frame->closure->fn->chunk,
				__frame_proc_idx_ext(cur_frame));

			assert(("closure object is not a function",
				OBJECT_IS_FN(fn)));

			lox_closure_t *closure =
				object_closure_new(OBJECT_AS_FN(fn));

			__vm_push_const(vm, VAL_CREATE_OBJ(closure));
		} break;

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
				__vm_runtime_error(vm,
						   "Operand types must match");
				return INTERPRET_RUNTIME_ERROR;
			}
			break;

		case OP_MOD: {
			if (!VAL_IS_NUMBER(*(__vm_peek_const_ptr(vm, 0))) ||
			    !VAL_IS_NUMBER(*(__vm_peek_const_ptr(vm, 1)))) {
				__vm_runtime_error(vm,
						   "Operand types must match");
				return INTERPRET_RUNTIME_ERROR;
			}

			lox_num_t b = __vm_pop_const(vm).as.number;
			lox_num_t a = __vm_pop_const(vm).as.number;
			__vm_push_const(vm, VAL_CREATE_NUMBER(fmod(a, b)));
		} break;

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
			break;

		case OP_EQUAL: {
			lox_val_t b = __vm_pop_const(vm);
			lox_val_t a = __vm_pop_const(vm);

			__vm_push_const(vm, VAL_CREATE_BOOL(val_equals(a, b)));
		} break;

		case OP_POP:
			__vm_pop_const(vm);
			break;

		case OP_POP_COUNT:
			__vm_discard(vm, __frame_proc_idx(cur_frame));
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

		case OP_UPVALUE_GET: {
			uint32_t slot = __frame_proc_idx(cur_frame);
			const lox_upval_t *upval = object_closure_get_upval(
				cur_frame->closure, slot);
			__vm_push_const(vm, *upval->location);
		} break;

		case OP_UPVALUE_GET_LONG: {
			uint32_t slot = __frame_proc_idx_ext(cur_frame);
			const lox_upval_t *upval = object_closure_get_upval(
				cur_frame->closure, slot);

			__vm_push_const(vm, *upval->location);
		} break;

		case OP_UPVALUE_SET: {
			uint32_t slot = __frame_proc_idx(cur_frame);
			lox_val_t *new_val = __vm_peek_const_ptr(vm, 0);

			object_closure_set_upval(cur_frame->closure, slot,
						 new_val);
		} break;

		case OP_UPVALUE_SET_LONG: {
			uint32_t slot = __frame_proc_idx_ext(cur_frame);
			lox_val_t *new_val = __vm_peek_const_ptr(vm, 0);

			object_closure_set_upval(cur_frame->closure, slot,
						 new_val);
		} break;

		case OP_GLOBAL_DEFINE: {
			uint32_t idx = __frame_proc_idx(cur_frame);
			lox_val_t *val = __vm_peek_const_ptr(vm, 0);

			__vm_define_global(vm, val, idx);
			__vm_pop_const(vm);
		} break;

		case OP_GLOBAL_DEFINE_LONG: {
			uint32_t idx = __frame_proc_idx(cur_frame);
			lox_val_t *val = __vm_peek_const_ptr(vm, 0);

			__vm_define_global(vm, val, idx);
			__vm_pop_const(vm);
		} break;

		case OP_GLOBAL_GET: {
			uint32_t idx = __frame_proc_idx(cur_frame);
			lox_val_t *val = __vm_get_global(vm, idx);

			if (!val) {
				__vm_runtime_error(vm, "undefined global.");
				return INTERPRET_RUNTIME_ERROR;
			}
			__vm_push_const(vm, *val);
		} break;

		case OP_GLOBAL_GET_LONG: {
			uint32_t idx = __frame_proc_idx_ext(cur_frame);
			lox_val_t *val = __vm_get_global(vm, idx);

			if (!val) {
				__vm_runtime_error(vm, "undefined global.");
				return INTERPRET_RUNTIME_ERROR;
			}
			__vm_push_const(vm, *val);
		} break;

		case OP_GLOBAL_SET: {
			uint32_t idx = __frame_proc_idx(cur_frame);
			lox_val_t *val = __vm_peek_const_ptr(vm, 0);

			if (!__vm_set_global(vm, idx, val)) {
				__vm_runtime_error(vm, "Undefined global.");
				return INTERPRET_RUNTIME_ERROR;
			}
		} break;

		case OP_GLOBAL_SET_LONG: {
			uint32_t idx = __frame_proc_idx_ext(cur_frame);
			lox_val_t *val = __vm_peek_const_ptr(vm, 0);

			if (!__vm_set_global(vm, idx, val)) {
				__vm_runtime_error(vm, "Undefined global.");
				return INTERPRET_RUNTIME_ERROR;
			}
		} break;

		case OP_VAR_DEFINE: {
			__frame_proc_idx(cur_frame);
			lox_val_t *val = __vm_peek_const_ptr(vm, 0);

			__vm_define_var(vm, val);
			__vm_pop_const(vm);
		} break;

		case OP_VAR_DEFINE_LONG: {
			__frame_proc_idx_ext(cur_frame);
			lox_val_t *val = __vm_peek_const_ptr(vm, 0);

			__vm_define_var(vm, val);
			__vm_pop_const(vm);
		} break;

		case OP_VAR_GET: {
			uint32_t idx = cur_frame->stack_snapshot +
				       __frame_proc_idx(cur_frame);
			lox_val_t *val = __vm_get_var(vm, idx);

			if (!val) {
				__vm_runtime_error(vm, "Undefined variable.");
				return INTERPRET_RUNTIME_ERROR;
			}
			__vm_push_const(vm, *val);
		} break;

		case OP_VAR_GET_LONG: {
			uint32_t idx = cur_frame->stack_snapshot +
				       __frame_proc_idx_ext(cur_frame);
			lox_val_t *val = __vm_get_var(vm, idx);

			if (!val) {
				__vm_runtime_error(vm, "Undefined variable.");
				return INTERPRET_RUNTIME_ERROR;
			}
			__vm_push_const(vm, *val);
		} break;

		case OP_VAR_SET: {
			uint32_t idx = cur_frame->stack_snapshot +
				       __frame_proc_idx(cur_frame);
			lox_val_t *val = __vm_peek_const_ptr(vm, 0);

			if (!__vm_set_var(vm, idx, val)) {
				__vm_runtime_error(vm, "Undefined variable.");
				return INTERPRET_RUNTIME_ERROR;
			}
		} break;

		case OP_VAR_SET_LONG: {
			uint32_t idx = cur_frame->stack_snapshot +
				       __frame_proc_idx_ext(cur_frame);
			lox_val_t *val = __vm_peek_const_ptr(vm, 0);

			if (!__vm_set_var(vm, idx, val)) {
				__vm_runtime_error(vm, "Undefined variable.");
				return INTERPRET_RUNTIME_ERROR;
			}
		} break;

		case OP_JUMP: {
			int16_t offset = __frame_proc_jump_offset(cur_frame);
			cur_frame->ip += offset;
		} break;

		case OP_JUMP_IF_FALSE: {
			int16_t offset = __frame_proc_jump_offset(cur_frame);

			if (val_is_falsey(__vm_peek_const(vm, 0))) {
				cur_frame->ip += offset;
			}
		} break;

		case OP_CALL: {
			uint8_t arg_cnt = __frame_proc_idx(cur_frame);

			if (!__vm_call_val(vm, __vm_peek_const(vm, arg_cnt),
					   arg_cnt)) {
				return INTERPRET_RUNTIME_ERROR;
			}

			cur_frame = list_peek(&vm->frames);
		} break;

		case OP_RETURN: {
			// arg_sz + 1
			list_pop(&vm->frames);

			if (!list_size(&vm->frames)) {
				__vm_pop_const(vm);
				return INTERPRET_OK;
			}

			lox_val_t retval = __vm_pop_const(vm);

			list_set_cnt(&vm->stack, cur_frame->stack_snapshot - 1);

			__vm_push_const(vm, retval);
			cur_frame = list_peek(&vm->frames);
		} break;

		default:
			printf("UNKNOWN OPCODE: %d\n", instr);
			return INTERPRET_RUNTIME_ERROR;
		}
#ifdef DEBUG_BENCH
		struct timespec time_end = timer_end(timer);
		struct timespec *prev_time =
			map_get(&vm->timings_map, op_name(instr));
		if (prev_time) {
			time_end = timespec_avg(*prev_time, time_end);
		}
		map_insert(&vm->timings_map, op_name(instr), &time_end);

#endif
	}

#undef BINARY_OP
#undef NUMERICAL_OP
#undef COMPARISON_OP
}

static void __vm_define_global(vm_t *vm, lox_val_t *val, size_t idx)
{
	if (idx >= list_size(&vm->globals)) {
		list_set_cnt(&vm->globals, idx + 1);
	}

	*((lox_val_t *)list_get(&vm->globals, idx)) = *val;
}

static lox_val_t *__vm_get_global(vm_t *vm, uint32_t glbl)
{
	if (glbl >= vm->globals.cnt) {
		return NULL;
	}

	return (lox_val_t *)list_get(&vm->globals, glbl);
}

static bool __vm_set_global(vm_t *vm, uint32_t glbl, lox_val_t *val)
{
	if (glbl >= vm->globals.cnt) {
		return false;
	}

	lox_val_t *val_ptr = (lox_val_t *)list_get(&vm->globals, glbl);
	memcpy(val_ptr, val, sizeof(lox_val_t));
	return true;
}
static void __vm_define_var(vm_t *vm, lox_val_t *val)
{
	list_push(&vm->stack, val);
}

static lox_val_t *__vm_get_var(vm_t *vm, uint32_t glbl)
{
	if (glbl >= vm->stack.cnt) {
		return NULL;
	}

	return (lox_val_t *)list_get(&vm->stack, glbl);
}

static bool __vm_set_var(vm_t *vm, uint32_t glbl, lox_val_t *val)
{
	if (glbl >= vm->stack.cnt) {
		return false;
	}

	lox_val_t *val_ptr = (lox_val_t *)list_get(&vm->stack, glbl);
	memcpy(val_ptr, val, sizeof(lox_val_t));
	return true;
}

static void __vm_set_main(vm_t *vm, lox_fn_t *main)
{
	lox_val_t main_obj = VAL_CREATE_OBJ(main);
	lox_closure_t *main_closure = object_closure_new(main);

	struct vm_call_frame main_frame = {
		.closure = main_closure,
		.ip = main->chunk.code.data,
		.stack_snapshot = STACK_RESERVED_COUNT,
	};

	if (!list_size(&vm->stack)) {
		list_push(&vm->stack, &main_obj);
	} else {
		*(lox_val_t *)list_get(&vm->stack, STACK_MAIN_IDX) = main_obj;
	}

	list_push(&vm->frames, &main_frame);
}

static uint32_t __frame_proc_idx(struct vm_call_frame *frame)
{
	__frame_assert_inst_ptr_valid(frame);
	return __frame_read_byte(frame);
}

static uint32_t __frame_proc_idx_ext(struct vm_call_frame *frame)
{
	__frame_assert_inst_ptr_valid(frame);
	uint32_t idx = *((uint32_t *)frame->ip) & EXT_CODE_MASK;
	frame->ip += EXT_CODE_SZ;

	return idx;
}

static int16_t __frame_proc_jump_offset(struct vm_call_frame *frame)
{
	__frame_assert_inst_ptr_valid(frame);
	int16_t idx = *((int16_t *)frame->ip);

	frame->ip += 2;

	return idx;
}

static void __vm_proc_const(vm_t *vm, struct vm_call_frame *frame, uint32_t idx)
{
	__frame_assert_inst_ptr_valid(frame);
	__vm_push_const(vm, chunk_get_const(&frame->closure->fn->chunk, idx));
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

	for (int i = list_size(&vm->frames) - 1; i >= 0; i--) {
		struct vm_call_frame *cur_frame = list_get(&vm->frames, i);
		lox_closure_t *closure = cur_frame->closure;

		size_t offset = ((size_t)__frame_instr_offset(cur_frame)) - 1;
		int line = chunk_get_line(&closure->fn->chunk, offset);

		fprintf(stderr, "[line %d] in %s()\n", line,
			closure->fn->name ? closure->fn->name->chars :
					    "script");
	}

	list_reset(&vm->stack);
}

static inline void
__frame_assert_inst_ptr_valid(const struct vm_call_frame *frame)
{
	assert(("Instruction pointer has passed code end",
		frame->ip < frame->closure->fn->chunk.code.data +
				    (frame->closure->fn->chunk.code.cnt *
				     frame->closure->fn->chunk.code.type_sz)));
}

static inline char __frame_read_byte(struct vm_call_frame *frame)
{
	return *frame->ip++;
}

static inline int __frame_instr_offset(const struct vm_call_frame *frame)
{
	return (int)(frame->ip - frame->closure->fn->chunk.code.data);
}

static void __vm_str_concat(vm_t *vm)
{
	const lox_str_t *b_str = OBJECT_AS_STRING(__vm_pop_const(vm));
	const lox_str_t *a_str = OBJECT_AS_STRING(__vm_pop_const(vm));
	lox_str_t *concat_str = object_str_concat(a_str, b_str);

	__vm_push_const(vm, VAL_CREATE_OBJ(concat_str));
}

static void __vm_discard(vm_t *vm, uint32_t discard_cnt)
{
	list_pop_bulk(&vm->stack, discard_cnt);
}

static bool __vm_call_val(vm_t *vm, lox_val_t callee, uint8_t call_arity)
{
	if (VAL_IS_OBJ(callee)) {
		switch (OBJECT_TYPE(callee)) {
		case OBJ_CLOSURE: {
			lox_closure_t *closure = OBJECT_AS_CLOSURE(callee);

			if (closure->fn->arity != call_arity) {
				__vm_runtime_error(
					vm,
					"Too %s arguments to function call, expected %d, have %d",
					closure->fn->arity > call_arity ?
						"few" :
						"many",
					closure->fn->arity, call_arity);
				return false;
			}

			if (list_size(&vm->frames) == CALL_FRAMES_MAX) {
				__vm_runtime_error(
					vm,
					"Stack overflow. Recursion depth of %d was reached",
					CALL_FRAMES_MAX);
				return false;
			}

			return __vm_call(vm, closure);
		}

		case OBJ_NATIVE: {
			const lox_native_t *native = OBJECT_AS_NATIVE(callee);

			lox_val_t res = native->fn(
				call_arity,
				call_arity ? list_peek_offset(&vm->stack,
							      call_arity - 1) :
					     NULL);

			if (VAL_IS_ERR(res)) {
				lox_val_t str = val_to_string(res);
				__vm_runtime_error(vm, OBJECT_AS_CSTRING(str));
				return false;
			}

			__vm_discard(vm, call_arity + 1);
			__vm_push_const(vm, res);

			return true;
		}

		default:
			assert(("Unknown value call type", 0));
			break;
		}
	}
	__vm_runtime_error(vm, "Can only call functions and classes");

	return false;
}

static bool __vm_call(vm_t *vm, lox_closure_t *closure)
{
	struct vm_call_frame frame = {
		.closure = closure,
		.ip = closure->fn->chunk.code.data,
		.stack_snapshot = list_size(&vm->stack) - closure->fn->arity,
	};
	list_push(&vm->frames, &frame);

	return true;
}

static lox_upval_t *__vm_capture_upval(vm_t *vm, size_t idx)
{
	list_t *stack = &vm->stack;
	lox_val_t *slot = list_get(stack, idx);

	lox_upval_t *new_upval = object_upval_new(slot);

	return new_upval;
}