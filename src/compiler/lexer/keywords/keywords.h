/**
 * @file keywords.h
 * @author Dylan Mayor
 * @brief header file for keyword parsing implementation
 *
 */
#ifndef __CLOX_LEXER_KEYWORDS_H__
#define __CLOX_LEXER_KEYWORDS_H__

#include "util/common.h"
#include "lexer/tokens.h"

/**
 * @brief traverses the given identifier and tries to return a matching keyword
 *
 * @param word the word to parse
 * @param word_sz the word length
 * @return enum tkn_type the matching keyword
 */
enum tkn_type keyword_traverse(const char *word, size_t word_sz);

#endif // __CLOX_LEXER_KEYWORDS_H__