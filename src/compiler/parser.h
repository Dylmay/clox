#ifndef __CLOX_COMPILER_PARSER_H__
#define __CLOX_COMPILER_PARSER_H__

#include "lexer/lexer.h"
#include "chunk/chunk.h"

typedef struct {
	lexer_t lexer;
	chunk_t *stack;
	token_t current;
	token_t previous;
	bool had_err;
	bool panic_mode;
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

#endif // __CLOX_COMPILER_PARSER_H__