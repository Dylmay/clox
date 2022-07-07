#include "parser.h"

#include <stdio.h>

bool parser_check(const parser_t *prsr, enum tkn_type type)
{
	return prsr->current.type == type;
}

bool parser_match(parser_t *prsr, enum tkn_type type)
{
	if (!parser_check(prsr, type)) {
		return false;
	}

	parser_advance(prsr);
	return true;
}

void parser_advance(parser_t *prsr)
{
	prsr->previous = prsr->current;

	while (true) {
		prsr->current = lexer_next_token(&prsr->lexer);

		if (prsr->current.type != TKN_ERR)
			break;

		parser_error_at_current(prsr, prsr->current.start);
	}
}

void parser_consume(parser_t *prsr, enum tkn_type tkn, const char *err_msg)
{
	if (prsr->current.type == tkn) {
		parser_advance(prsr);
		return;
	}

	parser_error_at_current(prsr, err_msg);
}

parser_t parser_new(const char *source, struct chunk *chunk,
		    struct state *state)
{
	lexer_t lexer = lexer_init(source);
	parser_t prsr = (parser_t){
		.lexer = lexer,
		.stack = chunk,
		.state = state,
	};

	parser_advance(&prsr);

	return prsr;
}

void parser_error(parser_t *prsr, const token_t *tkn, const char *msg)
{
	if (prsr->panic_mode) {
		return;
	}

	fprintf(stderr, "[line %d] Error", tkn->line);

	if (tkn->type == TKN_EOF) {
		fprintf(stderr, " at end");
	} else if (tkn->type != TKN_ERR) {
		fprintf(stderr, " at '%.*s'", (unsigned int)tkn->len,
			tkn->start);
	}

	fprintf(stderr, ": %s\n", msg);
	prsr->had_err = true;
	prsr->panic_mode = true;
}

void parser_sync(parser_t *prsr)
{
	prsr->panic_mode = false;

	while (!parser_check(prsr, TKN_EOF)) {
		if (parser_check(prsr, TKN_SEMICOLON)) {
			return;
		}

		switch (prsr->current.type) {
		case TKN_CLS:
		case TKN_FN:
		case TKN_LET:
		case TKN_FOR:
		case TKN_IF:
		case TKN_WHILE:
		case TKN_PRINT:
		case TKN_RET:
			return;

		default:
			break;
		}

		parser_advance(prsr);
	}
}
