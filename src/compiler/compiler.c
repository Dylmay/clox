#include <stdio.h>
#include <stdlib.h>

#include "util/common.h"
#include "compiler.h"
#include "compiler/parser/parser.h"
#include "ops/func/ops_func.h"
#include "chunk/func/chunk_func.h"
#include "val/func/val_func.h"
#include "val/func/object_func.h"

#if defined(DEBUG_PRINT_CODE) | defined(DEBUG_BENCH)
#include "debug/debug.h"
#endif

struct compile_unit {
	lox_fn_t *fn;
	bool can_assign;
};

// TODO: move to compile units. all share a pointer to the compiler struct
struct compiler {
	parser_t *prsr;
	struct state *state;
	// struct compile_unit *comp_unit;
	lox_fn_t *fn;
	bool can_assign;
};

typedef void (*parse_fn)(struct compiler *);

struct parse_rule {
	parse_fn prefix;
	parse_fn infix;
	enum precedence prec;
};

/* Forwards */
static lox_fn_t *__compiler_run(struct compiler, bool);
static const struct parse_rule *__compiler_get_rule(enum tkn_type);
static void __compiler_begin_scope(struct compiler *);
static void __compiler_end_scope(struct compiler *);
static lookup_var_t __compiler_define_var(struct compiler *, const char *,
					  size_t, int, bool);
static void __compiler_set_var(struct compiler *, lookup_var_t, int);
static void __compiler_get_var(struct compiler *, lookup_var_t, int);
static void __parse_block(struct compiler *);
static void __parse_expr(struct compiler *);
static void __parse_precedence(struct compiler *, enum precedence);
static void __parse_unary(struct compiler *);
static void __parse_binary(struct compiler *);
static void __parse_grouping(struct compiler *);
static void __parse_number(struct compiler *);
static void __parse_lit(struct compiler *);
static void __parse_string(struct compiler *);
static void __parse_decl(struct compiler *);
static void __parse_var_decl(struct compiler *);
static void __parse_stmnt(struct compiler *);
static void __parse_expr_stmt(struct compiler *);
static void __parse_print(struct compiler *);
static void __parse_var(struct compiler *);
static void __parse_if_stmt(struct compiler *);
static void __parse_while_stmt(struct compiler *);
static void __parse_and(struct compiler *);
static void __parse_or(struct compiler *);
static void __parse_for_stmt(struct compiler *);
static void __parse_fn_decl(struct compiler *);
static void __parse_fn(struct compiler *, lox_str_t *);
static void __parse_call(struct compiler *);
static void __parse_return_stmt(struct compiler *);
static uint8_t __parse_arglist(struct compiler *);
static bool __compiler_has_defined(struct compiler *, const char *, size_t);
/* Forwards */

static struct parse_rule PARSE_RULES[] = {
	// single char tokens
	[TKN_LEFT_PAREN] = { __parse_grouping, __parse_call, PREC_CALL },
	[TKN_RIGHT_PAREN] = { NULL, NULL, PREC_NONE },
	[TKN_LEFT_BRACE] = { NULL, NULL, PREC_NONE },
	[TKN_RIGHT_BRACE] = { NULL, NULL, PREC_NONE },
	[TKN_COMMA] = { NULL, NULL, PREC_NONE },
	[TKN_DOT] = { NULL, NULL, PREC_NONE },
	[TKN_MINUS] = { __parse_unary, __parse_binary, PREC_TERM },
	[TKN_PLUS] = { NULL, __parse_binary, PREC_TERM },
	[TKN_MOD] = { NULL, __parse_binary, PREC_FACTOR },
	[TKN_SEMICOLON] = { NULL, NULL, PREC_NONE },
	[TKN_SLASH] = { NULL, __parse_binary, PREC_FACTOR },
	[TKN_STAR] = { NULL, __parse_binary, PREC_FACTOR },
	[TKN_BANG] = { __parse_unary, NULL, PREC_NONE },
	[TKN_LESS] = { NULL, __parse_binary, PREC_COMPARISON },
	[TKN_GREATER] = { NULL, __parse_binary, PREC_COMPARISON },
	[TKN_EQ] = { NULL, NULL, PREC_NONE },
	// multi-char tokens
	[TKN_BANG_EQ] = { NULL, __parse_binary, PREC_EQUALITY },
	[TKN_EQ_EQ] = { NULL, __parse_binary, PREC_EQUALITY },
	[TKN_GREATER_EQ] = { NULL, __parse_binary, PREC_COMPARISON },
	[TKN_LESS_EQ] = { NULL, __parse_binary, PREC_COMPARISON },
	// Literals
	[TKN_ID] = { __parse_var, NULL, PREC_NONE },
	[TKN_STR] = { __parse_string, NULL, PREC_NONE },
	[TKN_NUM] = { __parse_number, NULL, PREC_NONE },
	// Keywords - 1
	[TKN_AND] = { NULL, __parse_and, PREC_AND },
	[TKN_CLS] = { NULL, NULL, PREC_NONE },
	[TKN_ELSE] = { NULL, NULL, PREC_NONE },
	[TKN_FOR] = { NULL, NULL, PREC_NONE },
	[TKN_FN] = { NULL, NULL, PREC_NONE },
	[TKN_IF] = { NULL, NULL, PREC_NONE },
	[TKN_NIL] = { __parse_lit, NULL, PREC_NONE },
	[TKN_OR] = { NULL, __parse_or, PREC_OR },
	[TKN_PRINT] = { __parse_print, NULL, PREC_NONE },
	[TKN_RETURN] = { NULL, NULL, PREC_NONE },
	[TKN_SUPER] = { NULL, NULL, PREC_NONE },
	[TKN_THIS] = { NULL, NULL, PREC_NONE },
	[TKN_TRUE] = { __parse_lit, NULL, PREC_NONE },
	[TKN_FALSE] = { __parse_lit, NULL, PREC_NONE },
	[TKN_LET] = { NULL, NULL, PREC_NONE },
	[TKN_WHILE] = { NULL, NULL, PREC_NONE },
	[TKN_ERR] = { NULL, NULL, PREC_NONE },
	[TKN_EOF] = { NULL, NULL, PREC_NONE },
};

static const struct parse_rule *__compiler_get_rule(enum tkn_type tkn)
{
	return &PARSE_RULES[tkn];
}

static struct compiler __compiler_new(parser_t *prsr, struct state *state,
				      lox_str_t *name)
{
	lox_fn_t *fn = object_fn_new();
	fn->name = name;

	return (struct compiler){
		.prsr = prsr,
		.state = state,
		.fn = fn,
		.can_assign = false,
	};
}

static lox_fn_t *__compiler_run(struct compiler compiler, bool is_main)
{
#ifdef DEBUG_BENCH
	struct timespec timer;
	timer_start(&timer);
#endif

	if (is_main) {
		while (!parser_match(compiler.prsr, TKN_EOF)) {
			__parse_decl(&compiler);
		}
	} else {
		__parse_block(&compiler);
	}

	// TODO: only push if no return was found - call to return leaves this as
	// dead code
	OP_CONST_WRITE(compiler.fn, VAL_CREATE_NIL,
		       compiler.prsr->current.line);
	OP_RETURN_WRITE(compiler.fn, compiler.prsr->current.line);

	if (parser_had_error(compiler.prsr)) {
		reallocate(compiler.fn, sizeof(struct chunk), 0);

		return NULL;
	} else {
#ifdef DEBUG_PRINT_CODE
		disassem_chunk(&compiler.fn->chunk,
			       compiler.fn->name != NULL ?
				       compiler.fn->name->chars :
				       "<script>");
#endif
#ifdef DEBUG_BENCH
		printf("Time taken to compile: ");
		timespec_print(timer_end(timer), true);
		puts("");
#endif
		return compiler.fn;
	}
}

lox_fn_t *compile(const char *src, struct state *state)
{
	parser_t prsr = parser_new(src);

	struct compiler compiler = __compiler_new(&prsr, state, NULL);

	return __compiler_run(compiler, true);
}

static void __parse_expr(struct compiler *compiler)
{
	__parse_precedence(compiler, PREC_ASSIGNMENT);
}

static void __parse_number(struct compiler *compiler)
{
	double val = strtod(compiler->prsr->previous.start, NULL);
	OP_CONST_WRITE(compiler->fn, VAL_CREATE_NUMBER(val),
		       compiler->prsr->previous.line);
}

static void __parse_grouping(struct compiler *compiler)
{
	__parse_expr(compiler);
	parser_consume(compiler->prsr, TKN_RIGHT_PAREN,
		       "Expect ')' after expression.");
}

static void __parse_unary(struct compiler *compiler)
{
	enum tkn_type tkn_type = compiler->prsr->previous.type;
	int line_num = compiler->prsr->previous.line;

	// compile operand
	__parse_precedence(compiler, PREC_UNARY);

	switch (tkn_type) {
	case TKN_MINUS:
		OP_NEGATE_WRITE(compiler->fn, line_num);
		break;

	case TKN_BANG:
		OP_NOT_WRITE(compiler->fn, line_num);
		break;

	default:
		assert(("Unexpected unary type", 0));
		return;
	}
}

static void __parse_binary(struct compiler *compiler)
{
	enum tkn_type tkn_type = compiler->prsr->previous.type;
	const struct parse_rule *rule = __compiler_get_rule(tkn_type);
	__parse_precedence(compiler, (enum precedence)(rule->prec + 1));

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
		assert(("Unexpected binary type", 0));
		return;
	}
}

static void __parse_precedence(struct compiler *compiler, enum precedence prec)
{
	parser_advance(compiler->prsr);
	parse_fn prefix_rule =
		__compiler_get_rule(compiler->prsr->previous.type)->prefix;

	if (!prefix_rule) {
		parser_error(compiler->prsr, compiler->prsr->previous,
			     "Expect expression.");
		return;
	}

	compiler->can_assign = prec <= PREC_ASSIGNMENT;
	prefix_rule(compiler);

	while (prec <=
	       __compiler_get_rule(compiler->prsr->current.type)->prec) {
		parser_advance(compiler->prsr);
		parse_fn infix_rule =
			__compiler_get_rule(compiler->prsr->previous.type)
				->infix;

		infix_rule(compiler);
	}

	if (compiler->can_assign && parser_match(compiler->prsr, TKN_EQ)) {
		parser_error_at_previous(compiler->prsr,
					 "Invalid assignment target.");
	}
}

static void __parse_lit(struct compiler *compiler)
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
		assert(("Unexpected literal type", 0));
		return;
	}
}

static void __parse_string(struct compiler *compiler)
{
	struct object_str *string =
		intern_string(&compiler->state->strings,
			      compiler->prsr->previous.start + 1,
			      compiler->prsr->previous.len - 2);

	OP_CONST_WRITE(compiler->fn, VAL_CREATE_OBJ(string),
		       compiler->prsr->previous.line);
}

static void __parse_decl(struct compiler *compiler)
{
	if (parser_match(compiler->prsr, TKN_LET)) {
		__parse_var_decl(compiler);
	} else if (parser_match(compiler->prsr, TKN_FN)) {
		__parse_fn_decl(compiler);
	} else {
		__parse_stmnt(compiler);
	}

	if (parser_in_panic_mode(compiler->prsr)) {
		parser_sync(compiler->prsr);
	}
}

static void __parse_var_decl(struct compiler *compiler)
{
	bool is_mutable = parser_match(compiler->prsr, TKN_MUT);
	parser_consume(compiler->prsr, TKN_ID, "Expected variable name");

	int def_ln = compiler->prsr->previous.line;
	const char *name = compiler->prsr->previous.start;
	size_t len = compiler->prsr->previous.len;

	if (__compiler_has_defined(compiler, name, len)) {
		parser_error_at_previous(compiler->prsr,
					 "Variables cannot be redefined.");
	}

	if (parser_match(compiler->prsr, TKN_EQ)) {
		__parse_expr(compiler);
	} else {
		OP_NIL_WRITE(compiler->fn, compiler->prsr->previous.line);
	}

	parser_consume(compiler->prsr, TKN_SEMICOLON,
		       "Expected ';' after variable declaration.");

	// if undef_var exists, set flags on object
	// could have a list of pendings
	if (!parser_had_error(compiler->prsr)) {
		__compiler_define_var(compiler, name, len, def_ln, is_mutable);
	} else {
		OP_VAR_DEFINE_WRITE(compiler->fn, 0, def_ln);
	}
}

static void __parse_stmnt(struct compiler *compiler)
{
	if (parser_match(compiler->prsr, TKN_PRINT)) {
		__parse_print(compiler);
	} else if (parser_match(compiler->prsr, TKN_IF)) {
		__parse_if_stmt(compiler);
	} else if (parser_match(compiler->prsr, TKN_RETURN)) {
		__parse_return_stmt(compiler);
	} else if (parser_match(compiler->prsr, TKN_WHILE)) {
		__parse_while_stmt(compiler);
	} else if (parser_match(compiler->prsr, TKN_FOR)) {
		__parse_for_stmt(compiler);
	} else if (parser_match(compiler->prsr, TKN_LEFT_BRACE)) {
		__compiler_begin_scope(compiler);
		__parse_block(compiler);
		__compiler_end_scope(compiler);
	} else {
		__parse_expr_stmt(compiler);
	}
}

static void __parse_block(struct compiler *compiler)
{
	while (!parser_check(compiler->prsr, TKN_RIGHT_BRACE) &&
	       !parser_check(compiler->prsr, TKN_EOF)) {
		__parse_decl(compiler);
	}

	parser_consume(compiler->prsr, TKN_RIGHT_BRACE,
		       "Expected '}' after block.");
}

static void __parse_expr_stmt(struct compiler *compiler)
{
	__parse_expr(compiler);
	parser_consume(compiler->prsr, TKN_SEMICOLON,
		       "Expected ';' after expression.");
	OP_POP_WRITE(compiler->fn, compiler->prsr->previous.line);
}

static void __parse_print(struct compiler *compiler)
{
	__parse_expr(compiler);
	parser_consume(compiler->prsr, TKN_SEMICOLON,
		       "Expected ';' after value.");
	OP_PRINT_WRITE(compiler->fn, compiler->prsr->previous.line);
}

static void __parse_var(struct compiler *compiler)
{
	token_t name_tkn = compiler->prsr->previous;
	lookup_var_t var = lookup_find_name(&compiler->state->lookup,
					    name_tkn.start, name_tkn.len);

	if (!lookup_var_is_declared(var)) {
		var = lookup_declare(&compiler->state->lookup,
				     LOOKUP_GLOBAL_DEPTH, name_tkn.start,
				     name_tkn.len, false);
	}

	if (compiler->can_assign && parser_match(compiler->prsr, TKN_EQ)) {
		__parse_expr(compiler);

		if (!lookup_var_is_mutable(var)) {
			parser_error(compiler->prsr, name_tkn,
				     "Variable isn't mutable");
		}

		__compiler_set_var(compiler, var,
				   compiler->prsr->previous.line);
	} else {
		__compiler_get_var(compiler, var,
				   compiler->prsr->previous.line);
	}
}

static void __parse_if_stmt(struct compiler *compiler)
{
	__parse_expr(compiler);

	if (!parser_check(compiler->prsr, TKN_LEFT_BRACE)) {
		parser_error_at_current(compiler->prsr,
					"Expected '{' or 'if' after else.");
	}

	int if_jump = OP_JUMP_IF_FALSE_WRITE(compiler->fn,
					     compiler->prsr->previous.line);
	OP_POP_WRITE(compiler->fn, compiler->prsr->previous.line);

	__parse_decl(compiler);

	int else_jump =
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

		__parse_decl(compiler);
	}

	if (!op_patch_jump(compiler->fn, else_jump)) {
		parser_error_at_current(compiler->prsr,
					"Too much code to jump over");
	}
}

static void __parse_and(struct compiler *compiler)
{
	int end_jump = OP_JUMP_IF_FALSE_WRITE(compiler->fn,
					      compiler->prsr->previous.line);
	OP_POP_WRITE(compiler->fn, compiler->prsr->previous.line);

	__parse_precedence(compiler, PREC_AND);
	op_patch_jump(compiler->fn, end_jump);
}

static void __parse_or(struct compiler *compiler)
{
	int else_jump = OP_JUMP_IF_FALSE_WRITE(compiler->fn,
					       compiler->prsr->previous.line);
	int end_jump =
		OP_JUMP_WRITE(compiler->fn, compiler->prsr->previous.line);

	op_patch_jump(compiler->fn, else_jump);
	OP_POP_WRITE(compiler->fn, compiler->prsr->previous.line);

	__parse_precedence(compiler, PREC_OR);
	op_patch_jump(compiler->fn, end_jump);
}

static void __parse_while_stmt(struct compiler *compiler)
{
	size_t loop_begin = chunk_cur_ip(&compiler->fn->chunk);
	__parse_expr(compiler);

	if (!parser_check(compiler->prsr, TKN_LEFT_BRACE)) {
		parser_error_at_current(compiler->prsr,
					"Expected '{' after condition.");
	}

	int exit_jump = OP_JUMP_IF_FALSE_WRITE(compiler->fn,
					       compiler->prsr->previous.line);
	OP_POP_WRITE(compiler->fn, compiler->prsr->previous.line);

	__parse_decl(compiler);

	OP_LOOP_WRITE(compiler->fn, loop_begin, compiler->prsr->previous.line);

	if (!op_patch_jump(compiler->fn, exit_jump)) {
		parser_error_at_current(compiler->prsr,
					"Too much code to jump over");
	}
	OP_POP_WRITE(compiler->fn, compiler->prsr->previous.line);

	if (parser_match(compiler->prsr, TKN_ELSE)) {
		__parse_stmnt(compiler);
	}
}

static void __parse_for_stmt(struct compiler *compiler)
{
	__compiler_begin_scope(compiler);
	parser_consume(compiler->prsr, TKN_ID, "Expected variable name");

	int def_ln = compiler->prsr->previous.line;
	const char *name = compiler->prsr->previous.start;
	size_t len = compiler->prsr->previous.len;

	if (__compiler_has_defined(compiler, name, len)) {
		parser_error_at_previous(compiler->prsr,
					 "Variable has already been defined.");
	}

	parser_consume(compiler->prsr, TKN_IN, "Expected 'in'.");
	parser_consume(compiler->prsr, TKN_NUM, "Expected range start");
	// write start of range
	double range_start = strtod(compiler->prsr->previous.start, NULL);
	OP_CONST_WRITE(compiler->fn, VAL_CREATE_NUMBER(range_start),
		       compiler->prsr->previous.line);
	lookup_var_t glbl_idx =
		__compiler_define_var(compiler, name, len, def_ln, true);

	parser_consume(compiler->prsr, TKN_DOT, "Expected range '..'");
	parser_consume(compiler->prsr, TKN_DOT, "Expected range '..'");
	parser_consume(compiler->prsr, TKN_NUM, "Expected range end");

	//condition
	double range_end = strtod(compiler->prsr->previous.start, NULL);

	int inc_start = chunk_cur_ip(&compiler->fn->chunk);
	OP_VAR_GET_WRITE(compiler->fn, glbl_idx.idx, def_ln);
	OP_CONST_WRITE(compiler->fn, VAL_CREATE_NUMBER(range_end),
		       compiler->prsr->previous.line);
	//LESS_EQ
	OP_LESS_WRITE(compiler->fn, compiler->prsr->previous.line);
	int exit_jump = OP_JUMP_IF_FALSE_WRITE(compiler->fn, def_ln);
	OP_POP_WRITE(compiler->fn, def_ln);

	if (!parser_check(compiler->prsr, TKN_LEFT_BRACE)) {
		parser_error_at_current(compiler->prsr, "Expected left brace");
	}
	// TODO: have unreleased scopes
	__parse_decl(compiler);

	OP_VAR_GET_WRITE(compiler->fn, glbl_idx.idx,
			 compiler->prsr->previous.line);
	OP_CONST_WRITE(compiler->fn, VAL_CREATE_NUMBER(1),
		       compiler->prsr->previous.line);
	OP_ADD_WRITE(compiler->fn, compiler->prsr->previous.line);
	__compiler_set_var(compiler, glbl_idx, compiler->prsr->previous.line);
	OP_POP_WRITE(compiler->fn, compiler->prsr->previous.line);
	OP_LOOP_WRITE(compiler->fn, inc_start, compiler->prsr->previous.line);

	op_patch_jump(compiler->fn, exit_jump);
	OP_POP_WRITE(compiler->fn, compiler->prsr->previous.line);
	__compiler_end_scope(compiler);
}

static void __parse_fn_decl(struct compiler *compiler)
{
	bool is_mutable = parser_match(compiler->prsr, TKN_MUT);

	parser_consume(compiler->prsr, TKN_ID, "Expected function name");

	const char *name = compiler->prsr->previous.start;
	size_t len = compiler->prsr->previous.len;

	if (__compiler_has_defined(compiler, name, len)) {
		parser_error_at_previous(
			compiler->prsr,
			"Variables/functions cannot be redefined.");
	}

	lox_str_t *fn_name =
		intern_string(&compiler->state->strings, name, len);
	__parse_fn(compiler, fn_name);

	if (!parser_had_error(compiler->prsr)) {
		__compiler_define_var(compiler, name, len,
				      compiler->prsr->previous.line,
				      is_mutable);

		lox_fn_t *fn = OBJECT_AS_FN(
			*(lox_val_t *)list_peek(&compiler->fn->chunk.consts));
		fn->name = fn_name;
	}
}

// NOTE: currently scopes args and internals separately
static void __parse_fn(struct compiler *compiler, lox_str_t *name)
{
	uint8_t arity = 0;
	struct compiler new_comp =
		__compiler_new(compiler->prsr, compiler->state, name);

	__compiler_begin_scope(&new_comp);
	parser_consume(new_comp.prsr, TKN_LEFT_PAREN,
		       "Expected '(' after function name.");
	if (!parser_check(compiler->prsr, TKN_RIGHT_PAREN)) {
		do {
			if (arity == 255) {
				parser_error_at_current(
					compiler->prsr,
					"Can't have more than 255 parameters.");
			}
			arity++;

			bool is_mutable = parser_match(compiler->prsr, TKN_MUT);
			parser_consume(compiler->prsr, TKN_ID,
				       "Expected variable name");

			int def_ln = compiler->prsr->previous.line;
			const char *name = compiler->prsr->previous.start;
			size_t len = compiler->prsr->previous.len;

			if (__compiler_has_defined(compiler, name, len)) {
				parser_error_at_previous(
					compiler->prsr,
					"Variables cannot be redefined.");
			}

			if (!parser_had_error(compiler->prsr)) {
				__compiler_define_var(&new_comp, name, len,
						      def_ln, is_mutable);
			}
		} while (parser_match(compiler->prsr, TKN_COMMA));
	}
	new_comp.fn->arity = arity;
	parser_consume(new_comp.prsr, TKN_RIGHT_PAREN,
		       "Expected ')' after function params.");
	parser_consume(new_comp.prsr, TKN_LEFT_BRACE, "Expected block start.");
	__compiler_run(new_comp, false);
	__compiler_end_scope(&new_comp);

	lox_fn_t *comp_res = new_comp.fn;
	OP_CONST_WRITE(compiler->fn, VAL_CREATE_OBJ(comp_res),
		       compiler->prsr->previous.line);
}

static void __parse_call(struct compiler *compiler)
{
	uint8_t args = __parse_arglist(compiler);
	OP_CALL_WRITE(compiler->fn, args, compiler->prsr->previous.line);
}

// TODO: as soon as reached, exit function compile
static void __parse_return_stmt(struct compiler *compiler)
{
	if (parser_match(compiler->prsr, TKN_SEMICOLON)) {
		//return nil
		OP_CONST_WRITE(compiler->fn, VAL_CREATE_NIL,
			       compiler->prsr->current.line);
	} else {
		// TODO: only allow number in top level
		__parse_expr(compiler);
		parser_consume(compiler->prsr, TKN_SEMICOLON,
			       "Expected ';' after return value.");
	}
	OP_RETURN_WRITE(compiler->fn, compiler->prsr->current.line);
}

static void __compiler_begin_scope(struct compiler *compiler)
{
	lookup_begin_scope(&compiler->state->lookup);
}

static void __compiler_end_scope(struct compiler *compiler)
{
	OP_POP_COUNT_WRITE(compiler->fn,
			   map_size(lookup_cur_scope(&compiler->state->lookup)),
			   compiler->prsr->current.line);

	lookup_end_scope(&compiler->state->lookup);
}

static lookup_var_t __compiler_define_var(struct compiler *compiler,
					  const char *name, size_t len,
					  int line, bool mutable)
{
	lookup_var_t new_var =
		lookup_define(&compiler->state->lookup, name, len, mutable);

	if (lookup_var_is_global(new_var)) {
		OP_GLOBAL_DEFINE_WRITE(compiler->fn, new_var.idx, line);
	} else {
		OP_VAR_DEFINE_WRITE(compiler->fn, new_var.idx, line);
	}

	return new_var;
}

static void __compiler_set_var(struct compiler *compiler, lookup_var_t var,
			       int line)
{
	if (lookup_var_is_global(var)) {
		OP_GLOBAL_SET_WRITE(compiler->fn, var.idx,
				    compiler->prsr->previous.line);
	} else {
		OP_VAR_SET_WRITE(compiler->fn, var.idx,
				 compiler->prsr->previous.line);
	}
}

static void __compiler_get_var(struct compiler *compiler, lookup_var_t var,
			       int line)
{
	if (lookup_var_is_global(var)) {
		OP_GLOBAL_GET_WRITE(compiler->fn, var.idx,
				    compiler->prsr->previous.line);
	} else {
		OP_VAR_GET_WRITE(compiler->fn, var.idx,
				 compiler->prsr->previous.line);
	}
}

static bool __compiler_has_defined(struct compiler *compiler, const char *name,
				   size_t len)
{
	return lookup_scope_has_name(&compiler->state->lookup, name, len);
}

static uint8_t __parse_arglist(struct compiler *compiler)
{
	uint8_t arg_cnt = 0;

	if (!parser_check(compiler->prsr, TKN_RIGHT_PAREN)) {
		do {
			__parse_expr(compiler);
			arg_cnt++;
		} while (parser_match(compiler->prsr, TKN_COMMA));
	}
	parser_consume(compiler->prsr, TKN_RIGHT_PAREN,
		       "Expected ')' after argument list");

	return arg_cnt;
}