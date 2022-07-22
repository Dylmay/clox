#include <stdio.h>
#include <string.h>

#include "lexer.h"
#include "keywords/keywords.h"
#include "util/common.h"
#include "util/char.h"

static bool __lexer_at_end(const lexer_t *lexer);
static char __lexer_advance(lexer_t *lexer);
static bool __lexer_match_char(lexer_t *lexer, char expected);
static void __lexer_skip_space(lexer_t *lexer);
static char __lexer_peek(const lexer_t *lexer);
static char __lexer_peek_next(const lexer_t *lexer);
static bool __lexer_consume_comment(lexer_t *lexer);
static token_t __lexer_str_token(lexer_t *lexer);
static token_t __lexer_id_token(lexer_t *lexer);
static token_t __lexer_num_token(lexer_t *lexer);

#define TOKEN(lexer, name)                                                     \
	((token_t){                                                            \
		.type = name,                                                  \
		.start = lexer->start,                                         \
		.len = (size_t)(lexer->current - lexer->start),                \
		.line = lexer->line,                                           \
	})

#define ERROR(lexer, msg)                                                      \
	((token_t){                                                            \
		.type = TKN_ERR,                                               \
		.start = msg,                                                  \
		.len = sizeof(msg) - 1,                                        \
		.line = lexer->line,                                           \
	})

lexer_t lexer_init(const char *src)
{
	return (lexer_t){
		.start = src,
		.current = src,
		.line = 1,
	};
}

token_t lexer_next_token(lexer_t *lexer)
{
	__lexer_skip_space(lexer);
	lexer->start = lexer->current;

	if (__lexer_at_end(lexer)) {
		return TOKEN(lexer, TKN_EOF);
	}

	char c = __lexer_advance(lexer);

	if (CHAR_IS_ALPHA(c)) {
		return __lexer_id_token(lexer);
	}

	if (CHAR_IS_DIGIT(c)) {
		token_t tkn = __lexer_num_token(lexer);
		return tkn;
	}

	switch (c) {
	case '(':
		return TOKEN(lexer, TKN_LEFT_PAREN);
	case ')':
		return TOKEN(lexer, TKN_RIGHT_PAREN);
	case '{':
		return TOKEN(lexer, TKN_LEFT_BRACE);
	case '}':
		return TOKEN(lexer, TKN_RIGHT_BRACE);
	case ';':
		return TOKEN(lexer, TKN_SEMICOLON);
	case ',':
		return TOKEN(lexer, TKN_COMMA);
	case '.':
		return TOKEN(lexer, TKN_DOT);
	case '-':
		return TOKEN(lexer, TKN_MINUS);
	case '+':
		return TOKEN(lexer, TKN_PLUS);
	case '/':
		return TOKEN(lexer, TKN_SLASH);
	case '*':
		return TOKEN(lexer, TKN_STAR);
	case '!':
		return TOKEN(lexer, __lexer_match_char(lexer, '=') ?
					    TKN_BANG_EQ :
					    TKN_BANG);
	case '=':
		return TOKEN(lexer, __lexer_match_char(lexer, '=') ? TKN_EQ_EQ :
								     TKN_EQ);
	case '<':
		return TOKEN(lexer, __lexer_match_char(lexer, '=') ?
					    TKN_LESS_EQ :
					    TKN_LESS);
	case '>':
		return TOKEN(lexer, __lexer_match_char(lexer, '=') ?
					    TKN_GREATER_EQ :
					    TKN_GREATER);

		break;
	case '"':
		return __lexer_str_token(lexer);
	default:
		break;
	}

	return ERROR(lexer, "Unexpected character");
}

static bool __lexer_at_end(const lexer_t *lexer)
{
	return *lexer->current == '\0';
}

static char __lexer_advance(lexer_t *lexer)
{
	lexer->current++;

	return lexer->current[-1];
}

static bool __lexer_match_char(lexer_t *lexer, char expected)
{
	if (__lexer_at_end(lexer)) {
		return false;
	}

	if (__lexer_peek(lexer) != expected) {
		return false;
	}

	lexer->current++;
	return true;
}

static void __lexer_skip_space(lexer_t *lexer)
{
	while (true) {
		char c = __lexer_peek(lexer);
		switch (c) {
		case '\n':
			lexer->line++;
			__lexer_advance(lexer);
			break;

		case ' ':
		case '\r':
		case '\t':
			__lexer_advance(lexer);
			break;
		case '/':
			if (!__lexer_consume_comment(lexer)) {
				return;
			}
			break;

		default:
			return;
		}
	}
}

static char __lexer_peek(const lexer_t *lexer)
{
	return *lexer->current;
}

static char __lexer_peek_next(const lexer_t *lexer)
{
	return __lexer_at_end(lexer) ? '\0' : lexer->current[1];
}

static bool __lexer_consume_comment(lexer_t *lexer)
{
	if (__lexer_peek_next(lexer) == '/') {
		while (__lexer_peek(lexer) != '\n' && __lexer_at_end(lexer)) {
			__lexer_advance(lexer);
		}
		return true;
	} else {
		return false;
	}
}

static token_t __lexer_str_token(lexer_t *lexer)
{
	while (__lexer_peek(lexer) != '"' && !__lexer_at_end(lexer)) {
		if (__lexer_peek(lexer) == '\n') {
			lexer->line++;
		}
		__lexer_advance(lexer);
	}

	if (__lexer_at_end(lexer)) {
		return ERROR(lexer, "Unterminated string.");
	}

	__lexer_advance(lexer);
	return TOKEN(lexer, TKN_STR);
}

static token_t __lexer_num_token(lexer_t *lexer)
{
	while (CHAR_IS_DIGIT(__lexer_peek(lexer))) {
		__lexer_advance(lexer);
	}

	if (__lexer_peek(lexer) == '.' && CHAR_IS_DIGIT(__lexer_peek(lexer))) {
		__lexer_advance(lexer);
	}

	while (CHAR_IS_DIGIT(__lexer_peek(lexer))) {
		__lexer_advance(lexer);
	}

	return TOKEN(lexer, TKN_NUM);
}

static token_t __lexer_id_token(lexer_t *lexer)
{
	while (CHAR_IS_ALPHA(__lexer_peek(lexer)) ||
	       CHAR_IS_DIGIT(__lexer_peek(lexer))) {
		__lexer_advance(lexer);
	}

	return TOKEN(lexer,
		     keyword_traverse(lexer->start,
				      (size_t)(lexer->current - lexer->start)));
}