/**
 * @file parser.h
 * @author Dylan Mayor
 * @brief header file for lox parser
 *
 */
#ifndef __CLOX_PARSER_H__
#define __CLOX_PARSER_H__

#include "util/common.h"
#include "lexer/lexer.h"
#include "val/val.h"

/**
 * @brief parser struct. Created using parser_new(). Contents must be freed after use by parser_free()
 *
 * @see parser_new()
 * @see parser_free()
 *
 */
typedef struct __parser {
	lexer_t lexer;
	token_t current;
	token_t previous;
	bool had_err;
	bool panic_mode;
} parser_t;

//! @brief parsing precedence levels
enum precedence {
	PREC_NONE,
	PREC_ASSIGNMENT, // =
	PREC_OR, // or
	PREC_AND, // and
	PREC_EQUALITY, // == !=
	PREC_COMPARISON, // < > <= >=
	PREC_TERM, // + -
	PREC_FACTOR, // * / mod
	PREC_UNARY, // ! -
	PREC_CALL, // . ()
	PREC_PRIMARY
};

/**
 * @brief creates a new lox parser
 *
 * @param source the source to parse. Must be addressable for the lifetime of the parser
 * @return parser_t the new parser
 */
parser_t parser_new(const char *source);

/**
 * @brief checks whether the current token is the given type
 *
 * @param prsr the parser
 * @param type the type to check
 * @return true the current token is of type 'type'
 * @return false the current token is not of type 'type'
 */
bool parser_check(const parser_t *prsr, enum tkn_type type);

/**
 * @brief advances the parser to the next token
 *
 * @param prsr the parser
 */
void parser_advance(parser_t *prsr);

/**
 * @brief checks whether the current token is the given type. If so, it advances the parser
 *
 * @see parser_advance()
 *
 * @param prsr the parser
 * @param type the type to check
 * @return true the current token is of type 'type' and the parser has advanced
 * @return false the current token is not of type 'type'
 */
static inline bool parser_match(parser_t *prsr, enum tkn_type type)
{
	if (parser_check(prsr, type)) {
		parser_advance(prsr);
		return true;
	}

	return false;
}

/**
 * @brief checks whether the current token is the given type and advances. The given error message is reported if the token is not the given type
 *
 * @param prsr the parser
 * @param tkn the expected token type
 * @param err_msg the error message
 */
void parser_consume(parser_t *prsr, enum tkn_type tkn, const char *err_msg);

/**
 * @brief reports a parser error message on the passed token. Sets the error flag and enables panic mode
 *
 * @param prsr the parser
 * @param tkn the token to report on
 * @param msg the error message to report
 */
void parser_error(parser_t *prsr, token_t tkn, const char *msg);

/**
 * @brief reports a parser error message on the current token
 *
 * @see parser_error()
 *
 * @param prsr the parser
 * @param msg the error message to report
 */
static inline void parser_error_at_current(parser_t *prsr, const char *msg)
{
	parser_error(prsr, prsr->current, msg);
}

/**
 * @brief reports a parser error message on the previously parsed/consumed token
 *
 * @see parser_error()
 *
 * @param prsr the parser
 * @param msg the error message to report
 */
static inline void parser_error_at_previous(parser_t *prsr, const char *msg)
{
	parser_error(prsr, prsr->previous, msg);
}

/**
 * @brief syncs the parser back to a known safe token and disables panic mode. Allows the parser to resume normal parsing after entering panic mode
 *
 * @param prsr the parser to sync
 */
void parser_sync(parser_t *prsr);

/**
 * @brief gets whether the passed parser is currently panicking.
 * can be cleared by calling parser_sync()
 *
 * @see parser_sync()
 *
 * @param prsr the parser
 * @return true the parser is currently panicking
 * @return false the parser is not currently panicking
 */
static inline bool parser_in_panic_mode(parser_t *prsr)
{
	return prsr->panic_mode;
}

/**
 * @brief gets whether the passed parser encountered an error when parsing
 *
 * @param prsr the parser
 * @return true the parser did encounter an error
 * @return false the parser hasn't encountered an error
 */
static inline bool parser_had_error(parser_t *prsr)
{
	return prsr->had_err;
}
#endif // __CLOX_PARSER_H__