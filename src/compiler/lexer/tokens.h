/**
 * @file tokens.h
 * @author Dylan Mayor
 * @brief  header file for lexer token definition
 *
 */
#ifndef __CLOX_SCANNER_TOKENS_H__
#define __CLOX_SCANNER_TOKENS_H__

//! @brief lox token types
enum tkn_type {
	// single char tokens
	TKN_LEFT_PAREN, // (
	TKN_RIGHT_PAREN, // )
	TKN_LEFT_BRACE, // {
	TKN_RIGHT_BRACE, // }
	TKN_COMMA, // ,
	TKN_DOT, // .
	TKN_MINUS, // -
	TKN_PLUS, // +
	TKN_SEMICOLON, // ;
	TKN_SLASH, // /
	TKN_STAR, // *
	TKN_BANG, // !
	TKN_LESS, // <
	TKN_GREATER, // >
	TKN_EQ, // =
	// multi-char tokens
	TKN_BANG_EQ, // !=
	TKN_EQ_EQ, // ==
	TKN_LESS_EQ, // <=
	TKN_GREATER_EQ, // >=
	// Literals
	TKN_ID, // {variable name}
	TKN_STR, // "..."
	TKN_NUM, // 0-9
	// Keywords
	TKN_MOD, // mod
	TKN_MUT, // mut
	TKN_AND, // and
	TKN_CLS, // cls
	TKN_ELSE, // else
	TKN_FALSE, // false
	TKN_FOR, // for
	TKN_IN, // in
	TKN_FN, // fn
	TKN_IF, // if
	TKN_NIL, // nil
	TKN_OR, // or
	TKN_PRINT, // print
	TKN_RETURN, // return
	TKN_SUPER, // super
	TKN_THIS, // this
	TKN_TRUE, // true
	TKN_WHILE, // while
	TKN_LET, // let
	TKN_ERR, // UNKNOWN
	TKN_EOF // END OF FILE
};

//! @brief item representing a lexed token
typedef struct __token {
	enum tkn_type type;
	const char *start;
	size_t len;
	int line;
} token_t;

#endif // __CLOX_SCANNER_TOKENS_H__