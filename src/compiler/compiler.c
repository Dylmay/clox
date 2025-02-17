#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include "util/common.h"
#include "compiler.h"
#include "compiler/parser/parser.h"
#include "ops/func/ops_func.h"
#include "chunk/func/chunk_func.h"
#include "val/func/val_func.h"
#include "val/func/object_func.h"
#include "util/dtoa.h"

#include "compiler/import.h"
#include "api/sys.h"

#if defined(DEBUG_PRINT_CODE) | defined(DEBUG_BENCH)
#include "debug/debug.h"
#endif

// TODO(dmayor): place check to stop recursive classes
// not possible due to flag being unset
enum define_state {
	DEFAULT_DEFINE,
	CLASS_DEFINE,
};

struct compiler {
	struct compiler *enclosing;
	vm_state_t *global_state;
	parser_t *prsr;
	struct {
		list_t scopes;
		uint32_t idx;
	} lookup;
	list_t upvalues;
	list_t captured_vals;
	lox_fn_t *fn;
	bool can_assign;
	enum define_state define_state;
};

typedef void (*parsing_fn)(struct compiler *);

struct parse_rule {
	parsing_fn prefix;
	parsing_fn infix;
	enum precedence prec;
};

/* Forwards */
static lox_fn_t *compiler_run(struct compiler *, bool);
static const struct parse_rule *compiler_get_rule(enum tkn_type);
static void compiler_begin_scope(struct compiler *);
static void compiler_end_scope(struct compiler *);
static lookup_var_t compiler_define_var(struct compiler *, const char *, size_t,
					uint32_t, bool);
static void compiler_set_var(struct compiler *, lookup_var_t);
static void compiler_get_var(struct compiler *, lookup_var_t);
static void parse_block(struct compiler *);
static void parse_expr(struct compiler *);
static void parse_precedence(struct compiler *, enum precedence);
static void parse_unary(struct compiler *);
static void parse_binary(struct compiler *);
static void parse_grouping(struct compiler *);
static double parse_number(struct compiler *);
static void parse_lit(struct compiler *);
static void parse_string(struct compiler *);
static void parse_class_decl(struct compiler *);
static void parse_decl(struct compiler *);
static void parse_var_decl(struct compiler *);
static void parse_stmnt(struct compiler *);
static void parse_expr_stmt(struct compiler *);
static void parse_var(struct compiler *);
static void parse_if_stmt(struct compiler *);
static void parse_while_stmt(struct compiler *);
static void parse_and(struct compiler *);
static void parse_or(struct compiler *);
static void parse_for_stmt(struct compiler *);
static void parse_fn_decl(struct compiler *);
static void parse_fn(struct compiler *, lox_str_t *);
static void parse_call(struct compiler *);
static void parse_return_stmt(struct compiler *);
static void parse_dot(struct compiler *);
static uint8_t parse_arglist(struct compiler *);
static bool compiler_has_defined(struct compiler *, const char *, size_t);
static void compiler_import(struct compiler *, native_import_list_t);
static lookup_var_t compiler_find_name(struct compiler *compiler,
				       const char *name, size_t name_sz);
static lookup_var_t compiler_find_name_local(struct compiler *compiler,
					     const char *name, size_t name_sz);
static lookup_t *compiler_cur_scope(struct compiler *);
static lookup_var_t compiler_resolve_upval(struct compiler *compiler,
					   const char *name, size_t name_sz);
static lookup_var_t compiler_add_upvalue(struct compiler *compiler,
					 lookup_var_t upval);
/* Forwards */

static struct parse_rule PARSE_RULES[] = {
	// single char tokens
	[TKN_LEFT_PAREN] = { parse_grouping, parse_call, PREC_CALL },
	[TKN_RIGHT_PAREN] = { NULL, NULL, PREC_NONE },
	[TKN_LEFT_BRACE] = { NULL, NULL, PREC_NONE },
	[TKN_RIGHT_BRACE] = { NULL, NULL, PREC_NONE },
	[TKN_COMMA] = { NULL, NULL, PREC_NONE },
	[TKN_DOT] = { NULL, parse_dot, PREC_CALL },
	[TKN_MINUS] = { parse_unary, parse_binary, PREC_TERM },
	[TKN_PLUS] = { parse_unary, parse_binary, PREC_TERM },
	[TKN_MOD] = { NULL, parse_binary, PREC_FACTOR },
	[TKN_SEMICOLON] = { NULL, NULL, PREC_NONE },
	[TKN_SLASH] = { NULL, parse_binary, PREC_FACTOR },
	[TKN_STAR] = { NULL, parse_binary, PREC_FACTOR },
	[TKN_BANG] = { parse_unary, NULL, PREC_NONE },
	[TKN_LESS] = { NULL, parse_binary, PREC_COMPARISON },
	[TKN_GREATER] = { NULL, parse_binary, PREC_COMPARISON },
	[TKN_EQ] = { NULL, NULL, PREC_NONE },
	// multi-char tokens
	[TKN_BANG_EQ] = { NULL, parse_binary, PREC_EQUALITY },
	[TKN_EQ_EQ] = { NULL, parse_binary, PREC_EQUALITY },
	[TKN_GREATER_EQ] = { NULL, parse_binary, PREC_COMPARISON },
	[TKN_LESS_EQ] = { NULL, parse_binary, PREC_COMPARISON },
	// Literals
	[TKN_ID] = { parse_var, NULL, PREC_NONE },
	[TKN_STR] = { parse_string, NULL, PREC_NONE },
	[TKN_NUM] = { (void (*)(struct compiler *))parse_number, NULL,
		      PREC_NONE },
	// Keywords - 1
	[TKN_AND] = { NULL, parse_and, PREC_AND },
	[TKN_CLS] = { NULL, NULL, PREC_NONE },
	[TKN_ELSE] = { NULL, NULL, PREC_NONE },
	[TKN_FOR] = { NULL, NULL, PREC_NONE },
	[TKN_FN] = { NULL, NULL, PREC_NONE },
	[TKN_IF] = { NULL, NULL, PREC_NONE },
	[TKN_NIL] = { parse_lit, NULL, PREC_NONE },
	[TKN_OR] = { NULL, parse_or, PREC_OR },
	[TKN_RETURN] = { NULL, NULL, PREC_NONE },
	[TKN_SUPER] = { NULL, NULL, PREC_NONE },
	[TKN_THIS] = { NULL, NULL, PREC_NONE },
	[TKN_TRUE] = { parse_lit, NULL, PREC_NONE },
	[TKN_FALSE] = { parse_lit, NULL, PREC_NONE },
	[TKN_LET] = { NULL, NULL, PREC_NONE },
	[TKN_WHILE] = { NULL, NULL, PREC_NONE },
	[TKN_ERR] = { NULL, NULL, PREC_NONE },
	[TKN_EOF] = { NULL, NULL, PREC_NONE },
};

static const struct parse_rule *compiler_get_rule(enum tkn_type tkn)
{
	return &PARSE_RULES[tkn];
}

static struct compiler compiler_new(struct compiler *enclosing, parser_t *prsr,
				    vm_state_t *state, lox_str_t *name)
{
	lox_fn_t *fn = object_fn_new(name);

	return (struct compiler){
		.enclosing = enclosing,
		.prsr = prsr,
		.global_state = state,
		.lookup = {
			.scopes = list_of_type(lookup_t),
			.idx = 0,
		},
		.upvalues = list_of_type(lookup_var_t),
		.captured_vals = list_of_type(uint32_t),
		.fn = fn,
		.can_assign = false,
		.define_state = DEFAULT_DEFINE,
	};
}

static lox_fn_t *compiler_run(struct compiler *compiler, bool is_main)
{
#ifdef DEBUG_BENCH
	struct timespec timer;
	timer_start(&timer);
#endif

	if (is_main) {
		compiler_import(compiler, sys_get_import_list());

		while (!parser_match(compiler->prsr, TKN_EOF)) {
			parse_decl(compiler);
		}
	} else {
		parse_block(compiler);
	}

	// TODO: only push if no return was found - call to return leaves this as
	// dead code
	OP_CONST_WRITE(compiler->fn, VAL_CREATE_NIL,
		       compiler->prsr->previous.line);
	OP_RETURN_WRITE(compiler->fn, compiler->prsr->previous.line);

	if (parser_had_error(compiler->prsr)) {
		reallocate(compiler->fn, sizeof(chunk_t), 0);

		return NULL;
	} else {
#ifdef DEBUG_PRINT_CODE
		disassem_chunk(&compiler->fn->chunk,
			       compiler->fn->name != NULL ?
				       compiler->fn->name->chars :
				       "<script>");
#endif
#ifdef DEBUG_BENCH
		printf("Time taken to compile: ");
		timespec_print(timer_end(timer), true);
		puts("");
#endif
		return compiler->fn;
	}
}

lox_fn_t *compile(const char *src, vm_state_t *state)
{
	parser_t prsr = parser_new(src);
	uint32_t global_sz = lookup_get_size(&state->globals);

	struct compiler compiler = compiler_new(NULL, &prsr, state, NULL);

	lox_fn_t *fn = compiler_run(&compiler, true);

	if (!fn) {
		uint32_t new_global_sz = lookup_get_size(&state->globals);

		for (uint32_t i = global_sz; i < new_global_sz; i++) {
			lookup_remove(&state->globals, i);
		}
	}

	return fn;
}

static void parse_expr(struct compiler *compiler)
{
	parse_precedence(compiler, PREC_ASSIGNMENT);
}

static double parse_number(struct compiler *compiler)
{
	char *end_ptr = NULL;
	errno = 0; // Reset errno before calling strtod
	double val = strtod(compiler->prsr->previous.start, &end_ptr);

	if (errno == ERANGE) {
		parser_error_at_previous(compiler->prsr, "Invalid number");
		errno = 0;
	}

	// TODO(dmayor): do we need this? This will throw if the size of the string does not match the number
	//  but, it's expected that this validation is done earlier
	// if (end_ptr != NULL && end_ptr - compiler->prsr->previous.start !=
	// 			       compiler->prsr->previous.len) {
	// 	parser_error_at_previous(compiler->prsr, "Invalid number");
	// }

	OP_CONST_WRITE(compiler->fn, VAL_CREATE_NUMBER(val),
		       compiler->prsr->previous.line);

	return val;
}

static void parse_grouping(struct compiler *compiler)
{
	parse_expr(compiler);
	parser_consume(compiler->prsr, TKN_RIGHT_PAREN,
		       "Expect ')' after expression.");
}

static void parse_unary(struct compiler *compiler)
{
	enum tkn_type tkn_type = compiler->prsr->previous.type;
	uint32_t line_num = compiler->prsr->previous.line;

	// compile operand
	parse_precedence(compiler, PREC_UNARY);

	switch (tkn_type) {
	case TKN_MINUS:
		OP_NEGATE_WRITE(compiler->fn, line_num);
		break;

	case TKN_BANG:
		OP_NOT_WRITE(compiler->fn, line_num);
		break;

	case TKN_PLUS:
		// no-op
		break;

	default:
		parser_error_at_current(compiler->prsr,
					"Unexpected unary type");
		return;
	}
}

static void parse_binary(struct compiler *compiler)
{
	enum tkn_type tkn_type = compiler->prsr->previous.type;
	const struct parse_rule *rule = compiler_get_rule(tkn_type);
	parse_precedence(compiler, (enum precedence)(rule->prec + 1));

	switch (tkn_type) {
	case TKN_BANG_EQ:
		OP_BANG_EQ_WRITE(compiler->fn, compiler->prsr->previous.line);
		break;
	case TKN_EQ_EQ:
		OP_EQUAL_WRITE(compiler->fn, compiler->prsr->previous.line);
		break;
	case TKN_GREATER:
		OP_GREATER_WRITE(compiler->fn, compiler->prsr->previous.line);
		break;
	case TKN_GREATER_EQ:
		OP_GREATER_EQ_WRITE(compiler->fn,
				    compiler->prsr->previous.line);
		break;
	case TKN_LESS:
		OP_LESS_WRITE(compiler->fn, compiler->prsr->previous.line);
		break;
	case TKN_LESS_EQ:
		OP_LESS_EQ_WRITE(compiler->fn, compiler->prsr->previous.line);
		break;
	case TKN_PLUS:
		OP_ADD_WRITE(compiler->fn, compiler->prsr->previous.line);
		break;
	case TKN_MINUS:
		OP_SUBTRACT_WRITE(compiler->fn, compiler->prsr->previous.line);
		break;
	case TKN_STAR:
		OP_MULTIPLY_WRITE(compiler->fn, compiler->prsr->previous.line);
		break;
	case TKN_SLASH:
		OP_DIVIDE_WRITE(compiler->fn, compiler->prsr->previous.line);
		break;
	case TKN_MOD:
		OP_MOD_WRITE(compiler->fn, compiler->prsr->previous.line);
		break;
	default:
		parser_error_at_current(compiler->prsr,
					"Unexpected binary type");
		return;
	}
}

static void parse_precedence(struct compiler *compiler, enum precedence prec)
{
	parser_advance(compiler->prsr);
	parsing_fn prefix_rule =
		compiler_get_rule(compiler->prsr->previous.type)->prefix;

	if (!prefix_rule) {
		parser_error(compiler->prsr, compiler->prsr->previous,
			     "Expect expression.");
		return;
	}

	compiler->can_assign = prec <= PREC_ASSIGNMENT;
	prefix_rule(compiler);

	while (prec <= compiler_get_rule(compiler->prsr->current.type)->prec) {
		parser_advance(compiler->prsr);
		parsing_fn infix_rule =
			compiler_get_rule(compiler->prsr->previous.type)->infix;

		infix_rule(compiler);
	}

	if (compiler->can_assign && parser_match(compiler->prsr, TKN_EQ)) {
		parser_error_at_previous(compiler->prsr,
					 "Invalid assignment target.");
	}
}

static void parse_lit(struct compiler *compiler)
{
	switch (compiler->prsr->previous.type) {
	case TKN_FALSE:
		OP_FALSE_WRITE(compiler->fn, compiler->prsr->previous.line);
		break;

	case TKN_TRUE:
		OP_TRUE_WRITE(compiler->fn, compiler->prsr->previous.line);
		break;

	case TKN_NIL:
		OP_NIL_WRITE(compiler->fn, compiler->prsr->previous.line);
		break;

	default:
		parser_error_at_current(compiler->prsr,
					"Unexpected literal type");
		return;
	}
}

static void parse_string(struct compiler *compiler)
{
	lox_str_t *string = object_str_new(compiler->prsr->previous.start + 1,
					   compiler->prsr->previous.len - 2);
	OP_CONST_WRITE(compiler->fn, VAL_CREATE_OBJ(string),
		       compiler->prsr->previous.line);
}

static void parse_decl(struct compiler *compiler)
{
	if (parser_match(compiler->prsr, TKN_CLS)) {
		parse_class_decl(compiler);
	} else if (parser_match(compiler->prsr, TKN_LET)) {
		parse_var_decl(compiler);
	} else if (parser_match(compiler->prsr, TKN_FN)) {
		parse_fn_decl(compiler);
	} else {
		parse_stmnt(compiler);
	}

	if (parser_in_panic_mode(compiler->prsr)) {
		parser_sync(compiler->prsr);
	}
}

static void parse_var_decl(struct compiler *compiler)
{
	bool is_mutable = parser_match(compiler->prsr, TKN_MUT);
	parser_consume(compiler->prsr, TKN_ID, "Expected variable name");

	uint32_t def_ln = compiler->prsr->previous.line;
	const char *name = compiler->prsr->previous.start;
	size_t len = compiler->prsr->previous.len;

	if (compiler_has_defined(compiler, name, len)) {
		parser_error_at_previous(compiler->prsr,
					 "Variables cannot be redefined.");
	}

	if (parser_match(compiler->prsr, TKN_EQ)) {
		parse_expr(compiler);
	} else {
		OP_NIL_WRITE(compiler->fn, compiler->prsr->previous.line);
	}

	parser_consume(compiler->prsr, TKN_SEMICOLON,
		       "Expected ';' after variable declaration.");

	// if undef_var exists, set flags on object
	// could have a list of pendings
	if (parser_had_error(compiler->prsr)) {
		OP_VAR_DEFINE_WRITE(compiler->fn, 0, def_ln);
	} else {
		compiler_define_var(compiler, name, len, def_ln, is_mutable);
	}
	// OP_POP_WRITE(compiler->fn, def_ln);
}

static void parse_stmnt(struct compiler *compiler)
{
	if (parser_match(compiler->prsr, TKN_IF)) {
		parse_if_stmt(compiler);
	} else if (parser_match(compiler->prsr, TKN_RETURN)) {
		parse_return_stmt(compiler);
	} else if (parser_match(compiler->prsr, TKN_WHILE)) {
		parse_while_stmt(compiler);
	} else if (parser_match(compiler->prsr, TKN_FOR)) {
		parse_for_stmt(compiler);
	} else if (parser_match(compiler->prsr, TKN_LEFT_BRACE)) {
		compiler_begin_scope(compiler);
		parse_block(compiler);
		compiler_end_scope(compiler);
	} else {
		parse_expr_stmt(compiler);
	}
}

static void parse_block(struct compiler *compiler)
{
	while (!parser_check(compiler->prsr, TKN_RIGHT_BRACE) &&
	       !parser_check(compiler->prsr, TKN_EOF)) {
		parse_decl(compiler);
	}

	parser_consume(compiler->prsr, TKN_RIGHT_BRACE,
		       "Expected '}' after block.");
}

static void parse_expr_stmt(struct compiler *compiler)
{
	parse_expr(compiler);
	parser_consume(compiler->prsr, TKN_SEMICOLON,
		       "Expected ';' after expression.");
	OP_POP_WRITE(compiler->fn, compiler->prsr->previous.line);
}

static void parse_var(struct compiler *compiler)
{
	token_t name_tkn = compiler->prsr->previous;
	lookup_var_t var =
		compiler_find_name(compiler, name_tkn.start, name_tkn.len);

	if (!lookup_var_is_declared(var)) {
		var = lookup_declare(
			&compiler->global_state->globals, name_tkn.start,
			name_tkn.len,
			lookup_get_size(&compiler->global_state->globals),
			LOOKUP_VAR_GLOBAL);
	}

	if (compiler->can_assign && parser_match(compiler->prsr, TKN_EQ)) {
		parse_expr(compiler);

		if (!lookup_var_is_mutable(var)) {
			parser_error(compiler->prsr, name_tkn,
				     "Variable isn't mutable");
		}

		compiler_set_var(compiler, var);
	} else {
		compiler_get_var(compiler, var);
	}
}

static void parse_if_stmt(struct compiler *compiler)
{
	parse_expr(compiler);

	if (!parser_check(compiler->prsr, TKN_LEFT_BRACE)) {
		parser_error_at_current(compiler->prsr,
					"Expected '{' or 'if' after else.");
	}

	size_t if_jump = OP_JUMP_IF_FALSE_WRITE(compiler->fn,
						compiler->prsr->previous.line);
	OP_POP_WRITE(compiler->fn, compiler->prsr->previous.line);

	parse_decl(compiler);

	size_t else_jump =
		OP_JUMP_WRITE(compiler->fn, compiler->prsr->previous.line);

	if (!op_patch_jump(compiler->fn, if_jump)) {
		parser_error_at_current(compiler->prsr,
					"Too much code to jump over");
	}
	OP_POP_WRITE(compiler->fn, compiler->prsr->previous.line);

	if (parser_match(compiler->prsr, TKN_ELSE)) {
		if (!parser_check(compiler->prsr, TKN_LEFT_BRACE) &&
		    !(parser_check(compiler->prsr, TKN_IF))) {
			parser_error_at_current(
				compiler->prsr,
				"Expected '{' after condition.");
		}

		parse_decl(compiler);
	}

	if (!op_patch_jump(compiler->fn, else_jump)) {
		parser_error_at_current(compiler->prsr,
					"Too much code to jump over");
	}
}

static void parse_and(struct compiler *compiler)
{
	size_t end_jump = OP_JUMP_IF_FALSE_WRITE(compiler->fn,
						 compiler->prsr->previous.line);
	OP_POP_WRITE(compiler->fn, compiler->prsr->previous.line);

	parse_precedence(compiler, PREC_AND);
	op_patch_jump(compiler->fn, end_jump);
}

static void parse_or(struct compiler *compiler)
{
	size_t else_jump = OP_JUMP_IF_FALSE_WRITE(
		compiler->fn, compiler->prsr->previous.line);
	size_t end_jump =
		OP_JUMP_WRITE(compiler->fn, compiler->prsr->previous.line);

	op_patch_jump(compiler->fn, else_jump);
	OP_POP_WRITE(compiler->fn, compiler->prsr->previous.line);

	parse_precedence(compiler, PREC_OR);
	op_patch_jump(compiler->fn, end_jump);
}

static void parse_while_stmt(struct compiler *compiler)
{
	size_t loop_begin = chunk_cur_instr(&compiler->fn->chunk);
	parse_expr(compiler);

	if (!parser_check(compiler->prsr, TKN_LEFT_BRACE)) {
		parser_error_at_current(compiler->prsr,
					"Expected '{' after condition.");
	}

	size_t exit_jump = OP_JUMP_IF_FALSE_WRITE(
		compiler->fn, compiler->prsr->previous.line);
	OP_POP_WRITE(compiler->fn, compiler->prsr->previous.line);

	parse_decl(compiler);

	OP_LOOP_WRITE(compiler->fn, loop_begin, compiler->prsr->previous.line);

	if (!op_patch_jump(compiler->fn, exit_jump)) {
		parser_error_at_current(compiler->prsr,
					"Too much code to jump over");
	}
	OP_POP_WRITE(compiler->fn, compiler->prsr->previous.line);

	if (parser_match(compiler->prsr, TKN_ELSE)) {
		parse_stmnt(compiler);
	}
}

static void parse_for_stmt(struct compiler *compiler)
{
	compiler_begin_scope(compiler);
	parser_consume(compiler->prsr, TKN_ID, "Expected variable name");

	uint32_t def_ln = compiler->prsr->previous.line;
	const char *name = compiler->prsr->previous.start;
	size_t len = compiler->prsr->previous.len;

	if (compiler_has_defined(compiler, name, len)) {
		parser_error_at_previous(compiler->prsr,
					 "Variable has already been defined.");
	}

	parser_consume(compiler->prsr, TKN_IN, "Expected 'in'");
	parser_consume(compiler->prsr, TKN_NUM, "Expected range start");
	// write start of range

	// TODO(dmayor): add support for statements
	double start_val = parse_number(compiler);

	if (start_val != floor(start_val)) {
		parser_error_at_previous(compiler->prsr,
					 "range number cannot be a decimal");
	}

	lookup_var_t glbl_idx =
		compiler_define_var(compiler, name, len, def_ln, false);

	parser_consume(compiler->prsr, TKN_DOT, "Expected range '..'");
	parser_consume(compiler->prsr, TKN_DOT, "Expected range '..'");
	parser_consume(compiler->prsr, TKN_NUM, "Expected range end");

	//condition

	size_t inc_start = chunk_cur_instr(&compiler->fn->chunk);
	OP_VAR_GET_WRITE(compiler->fn, glbl_idx.idx, def_ln);
	double end_val = parse_number(compiler);

	if (start_val != floor(start_val)) {
		parser_error_at_previous(compiler->prsr,
					 "range number cannot be a decimal");
	}
	//LESS_EQ
	OP_LESS_WRITE(compiler->fn, compiler->prsr->previous.line);
	size_t exit_jump = OP_JUMP_IF_FALSE_WRITE(compiler->fn, def_ln);
	OP_POP_WRITE(compiler->fn, def_ln);

	if (!parser_check(compiler->prsr, TKN_LEFT_BRACE)) {
		parser_error_at_current(compiler->prsr, "Expected left brace");
	}
	// TODO: stop new var from being created and use old var
	compiler_begin_scope(compiler);
	OP_VAR_GET_WRITE(compiler->fn, glbl_idx.idx,
			 compiler->prsr->previous.line);
	compiler_define_var(compiler, name, len, def_ln, false);
	parse_decl(compiler);
	compiler_end_scope(compiler);

	OP_VAR_GET_WRITE(compiler->fn, glbl_idx.idx,
			 compiler->prsr->previous.line);
	OP_CONST_WRITE(compiler->fn, VAL_CREATE_NUMBER(1),
		       compiler->prsr->previous.line);
	OP_ADD_WRITE(compiler->fn, compiler->prsr->previous.line);
	compiler_set_var(compiler, glbl_idx);
	OP_POP_WRITE(compiler->fn, compiler->prsr->previous.line);
	OP_LOOP_WRITE(compiler->fn, inc_start, compiler->prsr->previous.line);

	op_patch_jump(compiler->fn, exit_jump);
	OP_POP_WRITE(compiler->fn, compiler->prsr->previous.line);
	compiler_end_scope(compiler);
}

static void parse_fn_decl(struct compiler *compiler)
{
	bool is_mutable = parser_match(compiler->prsr, TKN_MUT);

	parser_consume(compiler->prsr, TKN_ID, "Expected function name");

	const char *name = compiler->prsr->previous.start;
	size_t len = compiler->prsr->previous.len;

	if (compiler_has_defined(compiler, name, len)) {
		parser_error_at_previous(
			compiler->prsr,
			"Variables/functions cannot be redefined.");
	}

	lox_str_t *fn_name = object_str_new(name, len);
	parse_fn(compiler, fn_name);

	if (!parser_had_error(compiler->prsr)) {
		compiler_define_var(compiler, name, len,
				    compiler->prsr->previous.line, is_mutable);

		lox_fn_t *fn = OBJECT_AS_FN(
			*(lox_val_t *)list_peek(&compiler->fn->chunk.consts));
		fn->name = fn_name;
	}
}

static void parse_fn(struct compiler *compiler, lox_str_t *name)
{
	uint8_t arity = 0;
	struct compiler new_comp = compiler_new(compiler, compiler->prsr,
						compiler->global_state, name);

	compiler_begin_scope(&new_comp);
	parser_consume(new_comp.prsr, TKN_LEFT_PAREN,
		       "Expected '(' after function name.");

	while (!parser_check(new_comp.prsr, TKN_RIGHT_PAREN)) {
		if (arity == 255) {
			parser_error_at_current(
				new_comp.prsr,
				"Can't have more than 255 parameters.");
		}
		arity++;

		bool is_mutable = parser_match(new_comp.prsr, TKN_MUT);

		parser_consume(new_comp.prsr, TKN_ID, "Expected variable name");

		uint32_t def_ln = new_comp.prsr->previous.line;
		const char *var_name = new_comp.prsr->previous.start;
		size_t len = new_comp.prsr->previous.len;

		if (compiler_has_defined(&new_comp, var_name, len)) {
			parser_error_at_previous(
				new_comp.prsr,
				"Variables cannot be redefined.");
		}

		if (!parser_had_error(new_comp.prsr)) {
			compiler_define_var(&new_comp, var_name, len, def_ln,
					    is_mutable);
		}

		if (!parser_match(new_comp.prsr, TKN_COMMA)) {
			break;
		}
	}

	new_comp.fn->arity = arity;
	parser_consume(new_comp.prsr, TKN_RIGHT_PAREN,
		       "Expected ')' after function params.");
	parser_consume(new_comp.prsr, TKN_LEFT_BRACE, "Expected block start.");
	lox_fn_t *comp_res = compiler_run(&new_comp, false);

	if (comp_res == NULL || parser_had_error(new_comp.prsr)) {
		return;
	}

	comp_res->upval_cnt = (uint32_t)list_size(&new_comp.upvalues);

	OP_CLOSURE_WRITE(compiler->fn, VAL_CREATE_OBJ(comp_res),
			 compiler->prsr->previous.line);

	for (uint32_t i = 0; i < comp_res->upval_cnt; i++) {
		lookup_var_t lookup_var =
			*(lookup_var_t *)list_get(&new_comp.upvalues, i);

		OP_UPVALUE_DEFINE_WRITE(compiler->fn, lookup_var.idx,
					lookup_var_is_global(lookup_var) ? 0 :
									   1,
					compiler->prsr->previous.line);
	}
}

static void parse_call(struct compiler *compiler)
{
	uint8_t args = parse_arglist(compiler);
	OP_CALL_WRITE(compiler->fn, args, compiler->prsr->previous.line);
}

// TODO: as soon as reached, exit function compile
static void parse_return_stmt(struct compiler *compiler)
{
	if (parser_match(compiler->prsr, TKN_SEMICOLON)) {
		//return nil
		OP_CONST_WRITE(compiler->fn, VAL_CREATE_NIL,
			       compiler->prsr->current.line);
	} else {
		// TODO: only allow number in top level
		parse_expr(compiler);
		parser_consume(compiler->prsr, TKN_SEMICOLON,
			       "Expected ';' after return value.");
	}
	OP_RETURN_WRITE(compiler->fn, compiler->prsr->current.line);
}

static void compiler_begin_scope(struct compiler *compiler)
{
	lookup_t new_scope = lookup_new();
	list_push(&compiler->lookup.scopes, &new_scope);
}

static void compiler_end_scope(struct compiler *compiler)
{
	lookup_t *cur_scope = compiler_cur_scope(compiler);
	uint32_t scope_sz = lookup_get_size(cur_scope);

	for (uint32_t i = compiler->lookup.idx - 1;
	     scope_sz > 0 && list_size(&compiler->captured_vals) != 0;
	     i--, scope_sz--) {
		uint32_t escaped_val =
			*(uint32_t *)list_peek(&compiler->captured_vals);

		if (i == escaped_val) {
			list_pop(&compiler->captured_vals);
			OP_CLOSE_UPVALUE_WRITE(compiler->fn,
					       compiler->prsr->previous.line);
		} else {
			OP_POP_WRITE(compiler->fn,
				     compiler->prsr->previous.line);
		}
	}

	if (scope_sz > 0) {
		OP_POP_COUNT_WRITE(compiler->fn, scope_sz,
				   compiler->prsr->previous.line);
	}
	compiler->lookup.idx -= lookup_get_size(cur_scope);
	lookup_free(cur_scope);
	list_pop(&compiler->lookup.scopes);
}

static lookup_var_t compiler_define_var(struct compiler *compiler,
					const char *name, size_t len,
					uint32_t line, bool mutable)
{
	// TODO: split in to two functions
	var_flags_t flags = mutable ? LOOKUP_VAR_MUTABLE : LOOKUP_VAR_IMMUTABLE;
	lookup_var_t new_var;

	if (compiler->enclosing == NULL &&
	    list_size(&compiler->lookup.scopes) == 0) {
		flags |= LOOKUP_VAR_GLOBAL;

		new_var = lookup_define(
			&compiler->global_state->globals, name, len,
			lookup_get_size(&compiler->global_state->globals),
			flags);

		if (!lookup_var_is_valid(new_var)) {
			parser_error_at_current(compiler->prsr,
						"Failed to define variable.");
		}
	} else {
		lookup_t *cur_scope = compiler_cur_scope(compiler);
		uint32_t prev_sz = lookup_get_size(cur_scope);

		new_var = lookup_define(cur_scope, name, len,
					compiler->lookup.idx, flags);

		if (lookup_get_size(cur_scope) > prev_sz) {
			compiler->lookup.idx++;
		}
	}

	if (compiler->define_state == CLASS_DEFINE) {
		// no op
		OP_PROPERTY_DEFINE_WRITE(compiler->fn, new_var.idx, line);
	} else if (lookup_var_is_global(new_var)) {
		OP_GLOBAL_DEFINE_WRITE(compiler->fn, new_var.idx, line);
	} else if (lookup_var_is_upval(new_var)) {
		parser_error_at_current(compiler->prsr,
					"Upval vars should never be defined.");
	} else {
		OP_VAR_DEFINE_WRITE(compiler->fn, new_var.idx, line);
	}

	return new_var;
}

static void compiler_set_var(struct compiler *compiler, lookup_var_t var)
{
	if (lookup_var_is_upval(var)) {
		OP_UPVALUE_SET_WRITE(compiler->fn, var.idx,
				     compiler->prsr->previous.line);
	} else if (lookup_var_is_global(var)) {
		OP_GLOBAL_SET_WRITE(compiler->fn, var.idx,
				    compiler->prsr->previous.line);
	} else {
		OP_VAR_SET_WRITE(compiler->fn, var.idx,
				 compiler->prsr->previous.line);
	}
}

static void compiler_get_var(struct compiler *compiler, lookup_var_t var)
{
	if (lookup_var_is_upval(var)) {
		OP_UPVALUE_GET_WRITE(compiler->fn, var.idx,
				     compiler->prsr->previous.line);
	} else if (lookup_var_is_global(var)) {
		OP_GLOBAL_GET_WRITE(compiler->fn, var.idx,
				    compiler->prsr->previous.line);
	} else {
		OP_VAR_GET_WRITE(compiler->fn, var.idx,
				 compiler->prsr->previous.line);
	}
}

static bool compiler_has_defined(struct compiler *compiler, const char *name,
				 size_t len)
{
	if (list_size(&compiler->lookup.scopes) == 0) {
		return lookup_has_name(&compiler->global_state->globals, name,
				       len);
	}

	const lookup_t *cur_scope = compiler_cur_scope(compiler);
	return lookup_has_name(cur_scope, name, len);
}

static void parse_class_decl(struct compiler *compiler)
{
	parser_consume(compiler->prsr, TKN_ID, "Expected class name");

	const char *name = compiler->prsr->previous.start;
	size_t len = compiler->prsr->previous.len;

	if (compiler_has_defined(compiler, name, len)) {
		parser_error_at_previous(
			compiler->prsr,
			"Variables/functions cannot be redefined.");
	}

	lox_str_t *cls_name = object_str_new(name, len);
	lox_class_t *cls = object_class_new(cls_name);

	OP_CONST_WRITE(compiler->fn, VAL_CREATE_OBJ(cls),
		       compiler->prsr->previous.line);
	// TODO: we don't define the var?
	/*lookup_var_t var =*/compiler_define_var(
		compiler, name, len, compiler->prsr->previous.line, false);
	list_push(&compiler->lookup.scopes, &cls->field_lookup);

	parser_consume(compiler->prsr, TKN_LEFT_BRACE,
		       "Expected '{' before class body");

	compiler->define_state = CLASS_DEFINE;
	while (!parser_check(compiler->prsr, TKN_RIGHT_BRACE) &&
	       !parser_check(compiler->prsr, TKN_EOF)) {
		if (parser_match(compiler->prsr, TKN_LET)) {
			parse_var_decl(compiler);
		} else if (parser_match(compiler->prsr, TKN_FN)) {
			parse_fn_decl(compiler);
		} else {
			parser_error_at_current(compiler->prsr,
						"Unknown class item");
			parser_advance(compiler->prsr);
		}
	}

	parser_consume(compiler->prsr, TKN_RIGHT_BRACE,
		       "Expected '}' after class body");
	compiler->define_state = DEFAULT_DEFINE;
	cls->field_lookup.table = *compiler_cur_scope(compiler);
	compiler->lookup.idx = 0;
	list_pop(&compiler->lookup.scopes);
}

static void parse_dot(struct compiler *compiler)
{
	parser_consume(compiler->prsr, TKN_ID, "Expecting a property name");
	token_t name_tkn = compiler->prsr->previous;
	lox_str_t *prop_name = object_str_new(name_tkn.start, name_tkn.len);

	// TODO: need to make this compile-time - Currently fails will be runtime
	if (compiler->can_assign && parser_match(compiler->prsr, TKN_EQ)) {
		parse_expr(compiler);

		// TODO: check for mutability - This is done at runtime currently
		// TODO: add ability to set defaults
		// TODO: add ability to set statics
		// if (!lookup_var_is_mutable(var)) {
		// 	parser_error(compiler->prsr, name_tkn,
		// 		     "Variable isn't mutable");
		// }
		OP_PROPERTY_SET_WRITE(compiler->fn, VAL_CREATE_OBJ(prop_name),
				      name_tkn.line);
		OP_POP_WRITE(compiler->fn, name_tkn.line);
	} else {
		OP_PROPERTY_GET_WRITE(compiler->fn, VAL_CREATE_OBJ(prop_name),
				      name_tkn.line);
	}
}

static uint8_t parse_arglist(struct compiler *compiler)
{
	uint8_t arg_cnt = 0;

	while (!parser_check(compiler->prsr, TKN_RIGHT_PAREN)) {
		parse_expr(compiler);
		arg_cnt++;

		if (!parser_match(compiler->prsr, TKN_COMMA)) {
			break;
		}
	}

	parser_consume(compiler->prsr, TKN_RIGHT_PAREN,
		       "Expected ')' after argument list");

	return arg_cnt;
}

static void compiler_import(struct compiler *compiler,
			    native_import_list_t imports)
{
	for (size_t i = 0; i < imports.import_cnt; i++) {
		native_import_t native_fn = imports.import_arr[i];

		if (compiler_has_defined(compiler, native_fn.fn_name,
					 native_fn.name_sz)) {
			// TODO: make it an error to overwrite an import
			// or properly support it
			continue;
		}

		uint32_t line = compiler->prsr->previous.line ?
					compiler->prsr->previous.line :
					1;

		OP_CONST_WRITE(compiler->fn,
			       VAL_CREATE_OBJ(object_native_fn_new(native_fn)),
			       line);

		compiler_define_var(compiler, native_fn.fn_name,
				    native_fn.name_sz, line, false);
	}
}

static lookup_var_t compiler_find_name(struct compiler *compiler,
				       const char *name, size_t name_sz)
{
	lookup_var_t var = compiler_find_name_local(compiler, name, name_sz);
	if (lookup_var_is_valid(var)) {
		return var;
	}

	var = compiler_resolve_upval(compiler, name, name_sz);
	if (lookup_var_is_valid(var)) {
		return var;
	}

	return lookup_find_name(&compiler->global_state->globals, name,
				name_sz);
}

static lookup_var_t compiler_find_name_local(struct compiler *compiler,
					     const char *name, size_t name_sz)
{
	lookup_var_t var = LOOKUP_VAR_TYPE_INVALID;

	if (list_size(&compiler->lookup.scopes) == 0) {
		return var;
	}

	for (size_t i = list_size(&compiler->lookup.scopes); i != 0; i--) {
		const lookup_t *cur_scope =
			list_get(&compiler->lookup.scopes, i - 1);

		var = lookup_find_name(cur_scope, name, name_sz);

		if (lookup_var_is_valid(var)) {
			return var;
		}
	}

	return var;
}

static lookup_var_t compiler_resolve_upval(struct compiler *compiler,
					   const char *name, size_t name_sz)
{
	if (compiler->enclosing == NULL) {
		return LOOKUP_VAR_TYPE_INVALID;
	}

	lookup_var_t local =
		compiler_find_name_local(compiler->enclosing, name, name_sz);
	if (lookup_var_is_valid(local)) {
		list_push(&compiler->enclosing->captured_vals, &local.idx);
		return compiler_add_upvalue(compiler, local);
	}

	lookup_var_t upval =
		compiler_resolve_upval(compiler->enclosing, name, name_sz);
	if (lookup_var_is_valid(upval)) {
		return compiler_add_upvalue(compiler, upval);
	}

	return LOOKUP_VAR_TYPE_INVALID;
}

static lookup_var_t compiler_add_upvalue(struct compiler *compiler,
					 lookup_var_t upval)
{
	upval.var_flags |= LOOKUP_VAR_UPVAL;
	upval.var_flags |= LOOKUP_VAR_LOCAL;

	uint32_t upval_cnt = (uint32_t)list_size(&compiler->upvalues);

	for (uint32_t i = 0; i < upval_cnt; i++) {
		lookup_var_t local_upval =
			*(lookup_var_t *)list_get(&compiler->upvalues, i);

		if (local_upval.idx == upval.idx &&
		    !lookup_var_is_global(upval)) {
			return (lookup_var_t){
				.idx = i,
				.var_flags = local_upval.var_flags |
					     LOOKUP_VAR_GLOBAL,
			};
		}
	}

	list_push(&compiler->upvalues, &upval);
	upval.idx = upval_cnt;

	return upval;
}

static lookup_t *compiler_cur_scope(struct compiler *compiler)
{
	return (lookup_t *)list_peek(&compiler->lookup.scopes);
}
