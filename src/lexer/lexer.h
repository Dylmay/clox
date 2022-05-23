#ifndef __CLOX_LEXER_H__
#define __CLOX_LEXER_H__

#include "tokens.h"

typedef struct {
	const char *start;
	const char *current;
	int line;
} lexer_t;

lexer_t lexer_init(const char *src);

token_t lexer_next_token(lexer_t *scanner);

#endif // __CLOX_LEXER_H__