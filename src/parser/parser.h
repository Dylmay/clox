#ifndef __CLOX_PARSER_H__
#define __CLOX_PARSER_H__

#include "util/common.h"
#include "lexer/lexer.h"
#include "chunk/chunk.h"

typedef struct {
	lexer_t lexer;
	chunk_t *stack;
	token_t current;
	token_t previous;
	bool had_err;
	bool panic_mode;
	bool can_assign;
} parser_t;

typedef void (*parse_fn)(parser_t *);

enum precedence {
	PREC_NONE,
	PREC_ASSIGNMENT, // =
	PREC_OR, // or
	PREC_AND, // and
	PREC_EQUALITY, // == !=
	PREC_COMPARISON, // < > <= >=
	PREC_TERM, // + -
	PREC_FACTOR, // * /
	PREC_UNARY, // ! -
	PREC_CALL, // . ()
	PREC_PRIMARY
};

struct parse_rule {
	parse_fn prefix;
	parse_fn infix;
	enum precedence prec;
};

parser_t parser_new(const char *source, chunk_t *chunk);
bool parser_check(const parser_t *prsr, enum tkn_type type);
bool parser_match(parser_t *prsr, enum tkn_type type);
void parser_advance(parser_t *prsr);
void parser_consume(parser_t *prsr, enum tkn_type tkn, const char *err_msg);
void parser_error(parser_t *prsr, const token_t *tkn, const char *msg);
void parser_sync(parser_t *prsr);

static inline void parser_error_at_current(parser_t *prsr, const char *msg)
{
	parser_error(prsr, &(prsr->current), msg);
}

static inline void parser_error_at_previous(parser_t *prsr, const char *msg)
{
	parser_error(prsr, &(prsr->previous), msg);
}

#endif // __CLOX_PARSER_H__