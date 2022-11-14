#include "parser.h"

#include <stdio.h>

/* Forwards */
static void __parser_unescape_string(parser_t *prsr);
/* Forwards */

bool parser_check(const parser_t *prsr, enum tkn_type type)
{
	return prsr->current.type == type;
}

void parser_advance(parser_t *prsr)
{
	prsr->previous = prsr->current;

	while (true) {
		prsr->current = lexer_next_token(&prsr->lexer);

		switch (prsr->current.type) {
		case TKN_COMMENT:
			continue;
		case TKN_ERR:
			parser_error_at_current(prsr, prsr->current.start);
			break;
		case TKN_STR:
			__parser_unescape_string(prsr);
			return;
		default:
			return;
		}
	}
}

void parser_consume(parser_t *prsr, enum tkn_type tkn, const char *err_msg)
{
	if (prsr->current.type == tkn) {
		parser_advance(prsr);
	} else {
		parser_error_at_current(prsr, err_msg);
	}
}

parser_t parser_new(const char *source)
{
	lexer_t lexer = lexer_init(source);
	parser_t prsr = (parser_t){
		.lexer = lexer,
	};

	parser_advance(&prsr);

	return prsr;
}

void parser_error(parser_t *prsr, token_t tkn, const char *msg)
{
	if (prsr->panic_mode) {
		return;
	}

	fprintf(stderr, "[line %d] Error", tkn.line);

	if (tkn.type == TKN_EOF) {
		fprintf(stderr, " at end");
	} else if (tkn.type != TKN_ERR) {
		fprintf(stderr, " at '%.*s'", (unsigned int)tkn.len, tkn.start);
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
		case TKN_RETURN:
			return;

		default:
			break;
		}

		parser_advance(prsr);
	}
}

// NOTE: string unescaping is destructive
// (saves creating a buffer for a known, valid string)
// if the escaped character is invalid, an error is reported so the
// string should not be present in the vm at runtime
// NOTE: also does not update the string token length so
// the string token will be out-of-sync for any escaped character strings
static void __parser_unescape_string(parser_t *prsr)
{
	size_t idx = 0;
	int offset = 0;
	char *chars = (char *)prsr->current.start + 1;
	size_t len = prsr->current.len - 1;

#define CHAR_OFFSET() (idx + offset)

	for (; CHAR_OFFSET() < len; idx++) {
#define HAS_NEXT_CHAR() (idx + offset + 1 < len)

		char cur_char = chars[CHAR_OFFSET()];

		if (cur_char == '\\' && HAS_NEXT_CHAR()) {
			offset++;
			char next_char = chars[CHAR_OFFSET()];
			switch (next_char) {
			case 'a':
				cur_char = '\a';
				break;
			case 'b':
				cur_char = '\b';
				break;
			case 'f':
				cur_char = '\f';
				break;
			case 'n':
				cur_char = '\n';
				break;
			case 'r':
				cur_char = '\r';
				break;
			case 't':
				cur_char = '\t';
				break;
			case 'v':
				cur_char = '\v';
				break;
			case '\\':
				cur_char = '\\';
				break;
			case '"':
				cur_char = '\"';
				break;
			default:
				parser_error_at_current(
					prsr, "Unknown escape sequence");
			}
		}
		chars[idx] = cur_char;

#undef HAS_NEXT_CHAR
	}

#undef CHAR_OFFSET

	prsr->current.len = idx + 1;
}