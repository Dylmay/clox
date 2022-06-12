#include <stdio.h>
#include <stdlib.h>

#include "util/common.h"
#include "compiler.h"
#include "parser/parser.h"
#include "ops/ops_func.h"
#include "val/func/val_func.h"
#include "val/func/object_func.h"
#include "lexer/lexer.h"

#ifdef DEBUG_PRINT_CODE
#include "debug/debug.h"
#endif

static const struct parse_rule *__compiler_get_rule(enum tkn_type);

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
	// Keywords
	[TKN_AND] = { NULL, NULL, PREC_NONE },
	[TKN_CLS] = { NULL, NULL, PREC_NONE },
	[TKN_ELSE] = { NULL, NULL, PREC_NONE },
	[TKN_FOR] = { NULL, NULL, PREC_NONE },
	[TKN_FN] = { NULL, NULL, PREC_NONE },
	[TKN_IF] = { NULL, NULL, PREC_NONE },
	[TKN_NIL] = { __parse_lit, NULL, PREC_NONE },
	[TKN_OR] = { NULL, NULL, PREC_NONE },
	[TKN_PRINT] = { __parse_print, NULL, PREC_NONE },
	[TKN_RET] = { NULL, NULL, PREC_NONE },
	[TKN_SUPER] = { NULL, NULL, PREC_NONE },
	[TKN_THIS] = { NULL, NULL, PREC_NONE },
	[TKN_TRUE] = { __parse_lit, NULL, PREC_NONE },
	[TKN_FALSE] = { __parse_lit, NULL, PREC_NONE },
	[TKN_VAR] = { NULL, NULL, PREC_NONE },
	[TKN_WHILE] = { NULL, NULL, PREC_NONE },
	[TKN_ERR] = { NULL, NULL, PREC_NONE },
	[TKN_EOF] = { NULL, NULL, PREC_NONE },
};

static const struct parse_rule *__compiler_get_rule(enum tkn_type tkn)
{
	return &PARSE_RULES[tkn];
}

bool compile(const char *src, chunk_t *chunk)
{
	parser_t prsr = parser_new(src, chunk);

	while (!parser_match(&prsr, TKN_EOF)) {
		__parse_decl(&prsr);
	}

	OP_RETURN_WRITE(chunk, prsr.current.line);
#ifdef DEBUG_PRINT_CODE
	if (!prsr.had_err) {
		disassem_chunk(chunk, "code");
	}
#endif
	return !prsr.had_err;
}

static void __parse_expr(parser_t *prsr)
{
	__parse_precedence(prsr, PREC_ASSIGNMENT);
}

static void __parse_number(parser_t *prsr)
{
	double val = strtod(prsr->previous.start, NULL);
	OP_CONST_WRITE(prsr->stack, VAL_CREATE_NUMBER(val),
		       prsr->previous.line);
}

static void __parse_grouping(parser_t *prsr)
{
	__parse_expr(prsr);
	parser_consume(prsr, TKN_RIGHT_PAREN,
				 "Expect ')' after expression.");
}

static void __parse_unary(parser_t *prsr)
{
	enum tkn_type tkn_type = prsr->previous.type;
	int line_num = prsr->current.line;

	// compile operand
	__parse_precedence(prsr, PREC_UNARY);

	switch (tkn_type) {
	case TKN_MINUS:
		OP_NEGATE_WRITE(prsr->stack, line_num);
		break;

	case TKN_BANG:
		OP_NOT_WRITE(prsr->stack, line_num);
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
		OP_EQUAL_WRITE(prsr->stack, prsr->previous.line);
		OP_NOT_WRITE(prsr->stack, prsr->previous.line);
		break;
	case TKN_EQ_EQ:
		OP_EQUAL_WRITE(prsr->stack, prsr->previous.line);
		break;
	case TKN_GREATER:
		OP_GREATER_WRITE(prsr->stack, prsr->previous.line);
		break;
	case TKN_GREATER_EQ:
		OP_LESS_WRITE(prsr->stack, prsr->previous.line);
		OP_NOT_WRITE(prsr->stack, prsr->previous.line);
		break;
	case TKN_LESS:
		OP_LESS_WRITE(prsr->stack, prsr->previous.line);
		break;
	case TKN_LESS_EQ:
		OP_GREATER_WRITE(prsr->stack, prsr->previous.line);
		OP_NOT_WRITE(prsr->stack, prsr->previous.line);
		break;
	case TKN_PLUS:
		OP_ADD_WRITE(prsr->stack, prsr->previous.line);
		break;
	case TKN_MINUS:
		OP_SUBTRACT_WRITE(prsr->stack, prsr->previous.line);
		break;
	case TKN_STAR:
		OP_MULTIPLY_WRITE(prsr->stack, prsr->previous.line);
		break;
	case TKN_SLASH:
		OP_DIVIDE_WRITE(prsr->stack, prsr->previous.line);
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

	prefix_rule(prsr);

	while (prec <= __compiler_get_rule(prsr->current.type)->prec) {
		parser_advance(prsr);
		__compiler_get_rule(prsr->previous.type)->infix(prsr);
	}
}

static void __parse_lit(parser_t *prsr)
{
	switch (prsr->previous.type) {
	case TKN_FALSE:
		OP_FALSE_WRITE(prsr->stack, prsr->previous.line);
		break;

	case TKN_TRUE:
		OP_TRUE_WRITE(prsr->stack, prsr->previous.line);
		break;

	case TKN_NIL:
		OP_NIL_WRITE(prsr->stack, prsr->previous.line);
		break;

	default:
		assert(("Unexpected literal type", 0));
		return;
	}
}

static void __parse_string(parser_t *prsr)
{
	struct object_str *string = chunk_intern_string(
		prsr->stack, prsr->previous.start + 1, prsr->previous.len - 2);

	OP_CONST_WRITE(prsr->stack, VAL_CREATE_OBJ(string),
		       prsr->previous.line);
}

static void __parse_decl(parser_t * prsr)
{
	if (parser_match(prsr, TKN_VAR)) {
		__parse_var_decl(prsr);
	} else {
    __parse_stmnt(prsr);
	}

	if (prsr->panic_mode) {
		parser_sync(prsr);
	}
}

static  void __parse_var_decl(parser_t *prsr)
{
	parser_consume(prsr, TKN_ID, "Expected variable name");

	int def_ln = prsr->previous.line;
	struct object_str *str = chunk_intern_string(
		prsr->stack, prsr->previous.start, prsr->previous.len);

	if (parser_match(prsr, TKN_EQ)) {
		__parse_expr(prsr);
	} else {
		OP_NIL_WRITE(prsr->stack, prsr->previous.line);
	}

	parser_consume(prsr, TKN_SEMICOLON,
		       "Expected ';' after variable declaration.");

	OP_GLOBAL_DEFINE_WRITE(prsr->stack, VAL_CREATE_OBJ(str), def_ln);
}

static void __parse_stmnt(parser_t *prsr)
{
	if (parser_match(prsr, TKN_PRINT)) {
		__parse_print(prsr);
	} else {
		__parse_expr_stmt(prsr);
	}
}

static void __parse_expr_stmt(parser_t *prsr)
{
	__parse_expr(prsr);
	parser_consume(prsr, TKN_SEMICOLON, "Expected ';' after expression.");
	OP_POP_WRITE(prsr->stack, prsr->previous.line);
}

static void __parse_print(parser_t *prsr)
{
	__parse_expr(prsr);
	parser_consume(prsr, TKN_SEMICOLON, "Expected ';' after value.");
	OP_PRINT_WRITE(prsr->stack, prsr->previous.line);
}

static void __parse_var(parser_t *prsr)
{
	struct object_str *str = chunk_intern_string(
		prsr->stack, prsr->previous.start, prsr->previous.len);

	OP_GLOBAL_GET_WRITE(prsr->stack, VAL_CREATE_OBJ(str), prsr->previous.line);
}
