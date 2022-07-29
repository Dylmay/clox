/**
 * @file lexer.h
 * @author Dylan Mayor
 * @brief header file for lox source lexer
 *
 */
#ifndef __CLOX_LEXER_H__
#define __CLOX_LEXER_H__

#include "tokens.h"
/**
 * @brief lox lexer struct. Created using lexer_init()
 *
 * @see lexer_init()
 *
 */
typedef struct __lexer {
	const char *start;
	const char *current;
	int line;
} lexer_t;

/**
 * @brief creates a new source lexer
 *
 * @param src the source
 * @return lexer_t the created lexer
 */
lexer_t lexer_init(const char *src);

/**
 * @brief moves the lexer forward across the source and returns the next token
 *
 * @param scanner the lexer
 * @return token_t the next token within the source
 */
token_t lexer_next_token(lexer_t *scanner);

#endif // __CLOX_LEXER_H__