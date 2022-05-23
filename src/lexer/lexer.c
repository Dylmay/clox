#include <stdio.h>
#include <string.h>

#include "lexer.h"
#include "keywords/keywords.h"
#include "util/common.h"
#include "util/char.h"

static bool _lexer_at_end(const lexer_t *lexer);
static char _lexer_advance(lexer_t *lexer);
static bool _lexer_match_char(lexer_t *lexer, char expected);
static void _lexer_skip_space(lexer_t *lexer);
static char _lexer_peek(const lexer_t *lexer);
static char _lexer_peek_next(const lexer_t *lexer);
static bool _lexer_consume_comment(lexer_t *lexer);
static token_t _lexer_str_token(lexer_t *lexer);
static token_t _lexer_id_token(lexer_t *lexer);
static token_t _lexer_num_token(lexer_t *lexer);

#define TOKEN(lexer, name)                                                     \
	((token_t){                                                            \
		type: name,                                                    \
		start: lexer->start,                                           \
		len: (int)*lexer->current - *lexer->start,                     \
		line: lexer->line                                              \
	})

#define ERROR(lexer, msg)                                                      \
	((token_t){                                                            \
		type: TKN_ERR,                                                 \
		start: msg,                                                    \
		len: sizeof(msg) - 1,                                          \
		line: lexer->line                                              \
	})

lexer_t lexer_init(const char *src)
{
	return (lexer_t){ start: src, current: src, line: 1 };
}

token_t lexer_next_token(lexer_t *lexer)
{
	lexer->start = lexer->current;

	if (_lexer_at_end(lexer)) {
		return TOKEN(lexer, TKN_EOF);
	}

	char c = _lexer_advance(lexer);

	if (CHAR_IS_ALPHA(c)) {
		return _lexer_id_token(lexer);
	}

	if (CHAR_IS_DIGIT(c)) {
		return _lexer_num_token(lexer);
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
		return TOKEN(lexer, _lexer_match_char(lexer, '=') ?
					    TKN_BANG_EQ :
						  TKN_BANG);
	case '=':
		return TOKEN(lexer, _lexer_match_char(lexer, '=') ? TKN_EQ_EQ :
									  TKN_EQ);
	case '<':
		return TOKEN(lexer, _lexer_match_char(lexer, '=') ?
					    TKN_LESS_EQ :
						  TKN_LESS);
	case '>':
		return TOKEN(lexer, _lexer_match_char(lexer, '=') ?
					    TKN_GREATER_EQ :
						  TKN_GREATER);

		break;
	case '"':
		return _lexer_str_token(lexer);
	}

	return ERROR(lexer, "Unexpected character");
}

static bool _lexer_at_end(const lexer_t *lexer)
{
	return *lexer->current == '\0';
}

static char _lexer_advance(lexer_t *lexer)
{
	lexer->current++;

	return lexer->current[-1];
}

static bool _lexer_match_char(lexer_t *lexer, char expected)
{
	if (_lexer_at_end(lexer)) {
		return false;
	}

	if (_lexer_peek(lexer) != expected) {
		return false;
	}

	lexer->current++;
	return true;
}

static void _lexer_skip_space(lexer_t *lexer)
{
	while (true) {
		char c = _lexer_peek(lexer);
		switch (c) {
		case '\n':
			lexer->line++;
			_lexer_advance(lexer);
			break;

		case ' ':
		case '\r':
		case '\t':
			_lexer_advance(lexer);
			break;
		case '/':
			if (!_lexer_consume_comment(lexer)) {
				return;
			}
			break;

		default:
			return;
		}
	}
}

static bool _lexer_consume_comment(lexer_t *lexer)
{
	if (_lexer_peek_next(lexer) == '/') {
		while (_lexer_peek(lexer) != '\n' && _lexer_at_end(lexer)) {
			_lexer_advance(lexer);
		}
		return true;
	} else {
		return false;
	}
}

static token_t _lexer_str_token(lexer_t *lexer)
{
	while (_lexer_peek(lexer) != '"' && !_lexer_at_end(lexer)) {
		if (_lexer_peek(lexer) == '\n') {
			lexer->line++;
		}
		_lexer_advance(lexer);
	}

	if (_lexer_at_end(lexer)) {
		return ERROR(lexer, "Unterminated string.");
	}

	_lexer_advance(lexer);
	return TOKEN(lexer, TKN_STR);
}

static token_t _lexer_num_token(lexer_t *lexer)
{
	while (CHAR_IS_DIGIT(_lexer_peek(lexer))) {
		_lexer_advance(lexer);
	}

	if (_lexer_peek(lexer) == '.' && CHAR_IS_DIGIT(_lexer_peek(lexer))) {
		_lexer_advance(lexer);
	}

	while (CHAR_IS_DIGIT(_lexer_peek(lexer))) {
		_lexer_advance(lexer);
	}

	return TOKEN(lexer, TKN_NUM);
}

static token_t _lexer_id_token(lexer_t *lexer)
{
	while (CHAR_IS_ALPHA(_lexer_peek(lexer)) ||
	       CHAR_IS_DIGIT(_lexer_peek(lexer))) {
		_lexer_advance(lexer);
	}

	return TOKEN(lexer,
		     keyword_traverse(lexer->start,
				      (int)(lexer->current - lexer->start)));
}