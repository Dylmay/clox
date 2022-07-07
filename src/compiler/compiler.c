#include <stdio.h>
#include <stdlib.h>

#include "util/common.h"
#include "compiler.h"
#include "compiler/parser/parser.h"
#include "ops/func/ops_func.h"
#include "chunk/func/chunk_func.h"
#include "val/func/val_func.h"
#include "val/func/object_func.h"
#include "compiler/lexer/lexer.h"

#if defined(DEBUG_PRINT_CODE) | defined(DEBUG_BENCH)
#include "debug/debug.h"
#endif

static const struct parse_rule *__compiler_get_rule(enum tkn_type);
static void __compiler_begin_scope(parser_t *prsr);
static void __compiler_end_scope(parser_t *prsr);

static void __parse_block(parser_t *prsr);
static void __parse_expr(parser_t *);
static void __parse_precedence(parser_t *, enum precedence);
static void __parse_unary(parser_t *);
static void __parse_binary(parser_t *);
static void __parse_grouping(parser_t *);
static void __parse_number(parser_t *);
static void __parse_lit(parser_t *);
static void __parse_string(parser_t *);
static void __parse_decl(parser_t *);
static void __parse_var_decl(parser_t *);
static void __parse_stmnt(parser_t *);
static void __parse_expr_stmt(parser_t *);
static void __parse_print(parser_t *);
static void __parse_var(parser_t *);
static void __parse_if_stmt(parser_t *);
static void __parse_while_stmt(parser_t *);
static void __parse_and(parser_t *);
static void __parse_or(parser_t *);
static void __parse_for_stmt(parser_t *);

static struct parse_rule PARSE_RULES[] = {
	// single char tokens
	[TKN_LEFT_PAREN] = { __parse_grouping, NULL, PREC_NONE },
	[TKN_RIGHT_PAREN] = { NULL, NULL, PREC_NONE },
	[TKN_LEFT_BRACE] = { NULL, NULL, PREC_NONE },
	[TKN_RIGHT_BRACE] = { NULL, NULL, PREC_NONE },
	[TKN_COMMA] = { NULL, NULL, PREC_NONE },
	[TKN_DOT] = { NULL, NULL, PREC_NONE },
	[TKN_MINUS] = { __parse_unary, __parse_binary, PREC_TERM },
	[TKN_PLUS] = { NULL, __parse_binary, PREC_TERM },
	[TKN_MOD] = { NULL, __parse_binary, PREC_TERM },
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
	[TKN_RET] = { NULL, NULL, PREC_NONE },
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

lox_fn_t *compile(const char *src, struct state *state)
{
	lox_fn_t *main = object_fn_new();

	parser_t prsr = parser_new(src, main, state);

#ifdef DEBUG_BENCH
	struct timespec timer;
	timer_start(&timer);
#endif

	while (!parser_match(&prsr, TKN_EOF)) {
		__parse_decl(&prsr);
	}

	OP_RETURN_WRITE(prsr.cur_fn, prsr.current.line);

	if (prsr.had_err) {
		reallocate(prsr.cur_fn, sizeof(struct chunk), 0);

		return NULL;
	} else {
#ifdef DEBUG_PRINT_CODE
		disassem_chunk(&main->chunk, "code");
#endif
#ifdef DEBUG_BENCH
		printf("Time taken to compile: ");
		timespec_print(timer_end(timer), true);
		puts("");
#endif
		return main;
	}
}

static void __parse_expr(parser_t *prsr)
{
	__parse_precedence(prsr, PREC_ASSIGNMENT);
}

static void __parse_number(parser_t *prsr)
{
	double val = strtod(prsr->previous.start, NULL);
	OP_CONST_WRITE(prsr->cur_fn, VAL_CREATE_NUMBER(val),
		       prsr->previous.line);
}

static void __parse_grouping(parser_t *prsr)
{
	__parse_expr(prsr);
	parser_consume(prsr, TKN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void __parse_unary(parser_t *prsr)
{
	enum tkn_type tkn_type = prsr->previous.type;
	int line_num = prsr->current.line;

	// compile operand
	__parse_precedence(prsr, PREC_UNARY);

	switch (tkn_type) {
	case TKN_MINUS:
		OP_NEGATE_WRITE(prsr->cur_fn, line_num);
		break;

	case TKN_BANG:
		OP_NOT_WRITE(prsr->cur_fn, line_num);
		break;

	default:
		assert(("Unexpected unary type", 0));
		return;
	}
}

static void __parse_binary(parser_t *prsr)
{
	enum tkn_type tkn_type = prsr->previous.type;
	const struct parse_rule *rule = __compiler_get_rule(tkn_type);
	__parse_precedence(prsr, (enum precedence)(rule->prec + 1));

	switch (tkn_type) {
	case TKN_BANG_EQ:
		OP_BANG_EQ_WRITE(prsr->cur_fn, prsr->previous.line);
		break;
	case TKN_EQ_EQ:
		OP_EQUAL_WRITE(prsr->cur_fn, prsr->previous.line);
		break;
	case TKN_GREATER:
		OP_GREATER_WRITE(prsr->cur_fn, prsr->previous.line);
		break;
	case TKN_GREATER_EQ:
		OP_GREATER_EQ_WRITE(prsr->cur_fn, prsr->previous.line);
		break;
	case TKN_LESS:
		OP_LESS_WRITE(prsr->cur_fn, prsr->previous.line);
		break;
	case TKN_LESS_EQ:
		OP_LESS_EQ_WRITE(prsr->cur_fn, prsr->previous.line);
		break;
	case TKN_PLUS:
		OP_ADD_WRITE(prsr->cur_fn, prsr->previous.line);
		break;
	case TKN_MINUS:
		OP_SUBTRACT_WRITE(prsr->cur_fn, prsr->previous.line);
		break;
	case TKN_STAR:
		OP_MULTIPLY_WRITE(prsr->cur_fn, prsr->previous.line);
		break;
	case TKN_SLASH:
		OP_DIVIDE_WRITE(prsr->cur_fn, prsr->previous.line);
		break;
	case TKN_MOD:
		OP_MOD_WRITE(prsr->cur_fn, prsr->previous.line);
		break;
	default:
		assert(("Unexpected binary type", 0));
		return;
	}
}

static void __parse_precedence(parser_t *prsr, enum precedence prec)
{
	parser_advance(prsr);
	parse_fn prefix_rule = __compiler_get_rule(prsr->previous.type)->prefix;

	if (!prefix_rule) {
		parser_error(prsr, &prsr->previous, "Expect expression.");
		return;
	}

	prsr->can_assign = prec <= PREC_ASSIGNMENT;
	prefix_rule(prsr);

	while (prec <= __compiler_get_rule(prsr->current.type)->prec) {
		parser_advance(prsr);
		parse_fn infix_rule =
			__compiler_get_rule(prsr->previous.type)->infix;

		infix_rule(prsr);
	}

	if (prsr->can_assign && parser_match(prsr, TKN_EQ)) {
		parser_error_at_previous(prsr, "Invalid assignment target.");
	}
}

static void __parse_lit(parser_t *prsr)
{
	switch (prsr->previous.type) {
	case TKN_FALSE:
		OP_FALSE_WRITE(prsr->cur_fn, prsr->previous.line);
		break;

	case TKN_TRUE:
		OP_TRUE_WRITE(prsr->cur_fn, prsr->previous.line);
		break;

	case TKN_NIL:
		OP_NIL_WRITE(prsr->cur_fn, prsr->previous.line);
		break;

	default:
		assert(("Unexpected literal type", 0));
		return;
	}
}

static void __parse_string(parser_t *prsr)
{
	struct object_str *string =
		intern_string(&prsr->state->strings, prsr->previous.start + 1,
			      prsr->previous.len - 2);

	OP_CONST_WRITE(prsr->cur_fn, VAL_CREATE_OBJ(string),
		       prsr->previous.line);
}

static void __parse_decl(parser_t *prsr)
{
	if (parser_match(prsr, TKN_LET)) {
		__parse_var_decl(prsr);
	} else {
		__parse_stmnt(prsr);
	}

	if (prsr->panic_mode) {
		parser_sync(prsr);
	}
}

static void __parse_var_decl(parser_t *prsr)
{
	bool is_mutable = parser_match(prsr, TKN_MUT);
	parser_consume(prsr, TKN_ID, "Expected variable name");

	int def_ln = prsr->previous.line;
	const char *name = prsr->previous.start;
	size_t len = prsr->previous.len;

	if (lookup_scope_has_name(&prsr->state->lookup, name, len)) {
		parser_error_at_previous(prsr,
					 "Variables cannot be redefined.");
	}

	if (parser_match(prsr, TKN_EQ)) {
		__parse_expr(prsr);
	} else {
		OP_NIL_WRITE(prsr->cur_fn, prsr->previous.line);
	}

	parser_consume(prsr, TKN_SEMICOLON,
		       "Expected ';' after variable declaration.");

	if (!prsr->had_err) {
		lookup_var_t glbl_idx = lookup_scope_define(
			&prsr->state->lookup, name, len,
			is_mutable ? LOOKUP_VAR_MUTABLE : LOOKUP_VAR_NO_FLAGS);

		OP_VAR_DEFINE_WRITE(prsr->cur_fn, glbl_idx.idx, def_ln);
	} else {
		OP_VAR_DEFINE_WRITE(prsr->cur_fn, 0, def_ln);
	}
}

static void __parse_stmnt(parser_t *prsr)
{
	if (parser_match(prsr, TKN_PRINT)) {
		__parse_print(prsr);
	} else if (parser_match(prsr, TKN_IF)) {
		__parse_if_stmt(prsr);
	} else if (parser_match(prsr, TKN_WHILE)) {
		__parse_while_stmt(prsr);
	} else if (parser_match(prsr, TKN_FOR)) {
		__parse_for_stmt(prsr);
	} else if (parser_match(prsr, TKN_LEFT_BRACE)) {
		__parse_block(prsr);
	} else {
		__parse_expr_stmt(prsr);
	}
}

static void __parse_block(parser_t *prsr)
{
	__compiler_begin_scope(prsr);
	while (!parser_check(prsr, TKN_RIGHT_BRACE) &&
	       !parser_check(prsr, TKN_EOF)) {
		__parse_decl(prsr);
	}

	parser_consume(prsr, TKN_RIGHT_BRACE, "Expected '}' after block.");
	__compiler_end_scope(prsr);
}

static void __parse_expr_stmt(parser_t *prsr)
{
	__parse_expr(prsr);
	parser_consume(prsr, TKN_SEMICOLON, "Expected ';' after expression.");
	OP_POP_WRITE(prsr->cur_fn, prsr->previous.line);
}

static void __parse_print(parser_t *prsr)
{
	__parse_expr(prsr);
	parser_consume(prsr, TKN_SEMICOLON, "Expected ';' after value.");
	OP_PRINT_WRITE(prsr->cur_fn, prsr->previous.line);
}

static void __parse_var(parser_t *prsr)
{
	token_t name_tkn = prsr->previous;

	lookup_var_t var = lookup_find_name(&prsr->state->lookup,
					    name_tkn.start, name_tkn.len);

	if (lookup_var_is_invalid(var)) {
		parser_error_at_previous(prsr, "Unknown identifier");
	}

	if (prsr->can_assign && parser_match(prsr, TKN_EQ)) {
		__parse_expr(prsr);

		if (!lookup_var_is_mutable(var)) {
			parser_error(prsr, &name_tkn, "Variable isn't mutable");
		}

		OP_VAR_SET_WRITE(prsr->cur_fn, var.idx, prsr->previous.line);
	} else {
		OP_VAR_GET_WRITE(prsr->cur_fn, var.idx, prsr->previous.line);
	}
}

static void __parse_if_stmt(parser_t *prsr)
{
	__parse_expr(prsr);

	if (!parser_check(prsr, TKN_LEFT_BRACE)) {
		parser_error_at_current(prsr,
					"Expected '{' or 'if' after else.");
	}

	int if_jump = OP_JUMP_IF_FALSE_WRITE(prsr->cur_fn, prsr->previous.line);
	OP_POP_WRITE(prsr->cur_fn, prsr->previous.line);

	__parse_decl(prsr);

	int else_jump = OP_JUMP_WRITE(prsr->cur_fn, prsr->previous.line);

	if (!op_patch_jump(prsr->cur_fn, if_jump)) {
		parser_error_at_current(prsr, "Too much code to jump over");
	}
	OP_POP_WRITE(prsr->cur_fn, prsr->previous.line);

	if (parser_match(prsr, TKN_ELSE)) {
		if (!parser_check(prsr, TKN_LEFT_BRACE) &&
		    !(parser_check(prsr, TKN_IF))) {
			parser_error_at_current(
				prsr, "Expected '{' after condition.");
		}

		__parse_decl(prsr);
	}

	if (!op_patch_jump(prsr->cur_fn, else_jump)) {
		parser_error_at_current(prsr, "Too much code to jump over");
	}
}

static void __parse_and(parser_t *prsr)
{
	int end_jump =
		OP_JUMP_IF_FALSE_WRITE(prsr->cur_fn, prsr->previous.line);
	OP_POP_WRITE(prsr->cur_fn, prsr->previous.line);

	__parse_precedence(prsr, PREC_AND);
	op_patch_jump(prsr->cur_fn, end_jump);
}

static void __parse_or(parser_t *prsr)
{
	int else_jump =
		OP_JUMP_IF_FALSE_WRITE(prsr->cur_fn, prsr->previous.line);
	int end_jump = OP_JUMP_WRITE(prsr->cur_fn, prsr->previous.line);

	op_patch_jump(prsr->cur_fn, else_jump);
	OP_POP_WRITE(prsr->cur_fn, prsr->previous.line);

	__parse_precedence(prsr, PREC_OR);
	op_patch_jump(prsr->cur_fn, end_jump);
}

static void __parse_while_stmt(parser_t *prsr)
{
	size_t loop_begin = chunk_cur_ip(&prsr->cur_fn->chunk);
	__parse_expr(prsr);

	if (!parser_check(prsr, TKN_LEFT_BRACE)) {
		parser_error_at_current(prsr, "Expected '{' after condition.");
	}

	int exit_jump =
		OP_JUMP_IF_FALSE_WRITE(prsr->cur_fn, prsr->previous.line);
	OP_POP_WRITE(prsr->cur_fn, prsr->previous.line);

	__parse_decl(prsr);

	OP_LOOP_WRITE(prsr->cur_fn, loop_begin, prsr->previous.line);

	if (!op_patch_jump(prsr->cur_fn, exit_jump)) {
		parser_error_at_current(prsr, "Too much code to jump over");
	}
	OP_POP_WRITE(prsr->cur_fn, prsr->previous.line);

	if (parser_match(prsr, TKN_ELSE)) {
		__parse_stmnt(prsr);
	}
}

static void __parse_for_stmt(parser_t *prsr)
{
	__compiler_begin_scope(prsr);
	// int loop_start = chunk_cur_ip(prsr->stack);
	parser_consume(prsr, TKN_ID, "Expected variable name");

	int def_ln = prsr->previous.line;
	const char *name = prsr->previous.start;
	size_t len = prsr->previous.len;

	if (lookup_scope_has_name(&prsr->state->lookup, name, len)) {
		parser_error_at_previous(prsr,
					 "Variable has already been defined.");
	}

	parser_consume(prsr, TKN_IN, "Expected 'in'.");
	parser_consume(prsr, TKN_NUM, "Expected range start");
	// write start of range
	double range_start = strtod(prsr->previous.start, NULL);
	OP_CONST_WRITE(prsr->cur_fn, VAL_CREATE_NUMBER(range_start),
		       prsr->previous.line);
	lookup_var_t glbl_idx = lookup_scope_define(&prsr->state->lookup, name,
						    len, LOOKUP_VAR_MUTABLE);
	OP_VAR_DEFINE_WRITE(prsr->cur_fn, glbl_idx.idx, def_ln);

	parser_consume(prsr, TKN_DOT, "Expected range '..'");
	parser_consume(prsr, TKN_DOT, "Expected range '..'");
	parser_consume(prsr, TKN_NUM, "Expected range end");

	//condition
	double range_end = strtod(prsr->previous.start, NULL);

	int inc_start = chunk_cur_ip(&prsr->cur_fn->chunk);
	OP_VAR_GET_WRITE(prsr->cur_fn, glbl_idx.idx, def_ln);
	OP_CONST_WRITE(prsr->cur_fn, VAL_CREATE_NUMBER(range_end),
		       prsr->previous.line);
	//LESS_EQ
	OP_LESS_WRITE(prsr->cur_fn, prsr->previous.line);
	int exit_jump = OP_JUMP_IF_FALSE_WRITE(prsr->cur_fn, def_ln);
	OP_POP_WRITE(prsr->cur_fn, def_ln);

	if (!parser_check(prsr, TKN_LEFT_BRACE)) {
		parser_error_at_current(prsr, "Expected left brace");
	}
	// TODO: have unreleased scopes
	__parse_decl(prsr);

	OP_VAR_GET_WRITE(prsr->cur_fn, glbl_idx.idx, prsr->previous.line);
	OP_CONST_WRITE(prsr->cur_fn, VAL_CREATE_NUMBER(1), prsr->previous.line);
	OP_ADD_WRITE(prsr->cur_fn, prsr->previous.line);
	OP_VAR_SET_WRITE(prsr->cur_fn, glbl_idx.idx, prsr->previous.line);
	OP_POP_WRITE(prsr->cur_fn, prsr->previous.line);
	OP_LOOP_WRITE(prsr->cur_fn, inc_start, prsr->previous.line);

	op_patch_jump(prsr->cur_fn, exit_jump);
	OP_POP_WRITE(prsr->cur_fn, prsr->previous.line);
	__compiler_end_scope(prsr);
}

static void __compiler_begin_scope(parser_t *prsr)
{
	lookup_begin_scope(&prsr->state->lookup);
}

static void __compiler_end_scope(parser_t *prsr)
{
	OP_POP_COUNT_WRITE(prsr->cur_fn,
			   map_size(lookup_cur_scope(&prsr->state->lookup)),
			   prsr->previous.line);
	lookup_end_scope(&prsr->state->lookup);
}