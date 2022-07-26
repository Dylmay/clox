#ifndef __CLOX_SCANNER_TOKENS_H__
#define __CLOX_SCANNER_TOKENS_H__

enum tkn_type {
	// single char tokens
	TKN_LEFT_PAREN,
	TKN_RIGHT_PAREN,
	TKN_LEFT_BRACE,
	TKN_RIGHT_BRACE,
	TKN_COMMA,
	TKN_DOT,
	TKN_MINUS,
	TKN_PLUS,
	TKN_SEMICOLON,
	TKN_SLASH,
	TKN_STAR,
	TKN_BANG,
	TKN_LESS,
	TKN_GREATER,
	TKN_EQ,
	// multi-char tokens
	TKN_BANG_EQ,
	TKN_EQ_EQ,
	TKN_LESS_EQ,
	TKN_GREATER_EQ,
	// Literals
	TKN_IN,
	TKN_ID,
	TKN_STR,
	TKN_NUM,
	// Keywords
	TKN_MOD,
	TKN_MUT,
	TKN_AND,
	TKN_CLS,
	TKN_ELSE,
	TKN_FALSE,
	TKN_FOR,
	TKN_FN,
	TKN_IF,
	TKN_NIL,
	TKN_OR,
	TKN_PRINT,
	TKN_RETURN,
	TKN_SUPER,
	TKN_THIS,
	TKN_TRUE,
	TKN_WHILE,
	TKN_LET,
	TKN_ERR,
	TKN_EOF
};

typedef struct {
	enum tkn_type type;
	const char *start;
	size_t len;
	int line;
} token_t;

#endif // __CLOX_SCANNER_TOKENS_H__