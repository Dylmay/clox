/**
 * @file compiler.h
 * @author Dylan Mayor
 * @brief header file holding the lox compiler implementation
 *
 */
#ifndef __CLOX_COMPILER_H__
#define __CLOX_COMPILER_H__

#include "chunk/chunk.h"
#include "state/state.h"
#include "val/val.h"

/**
 * @brief compiles the given source in to a script function
 *
 * @param src the source to compile
 * @param state the current vm state
 * @return lox_fn_t* the root script function. NULL if an error was encountered
 */
lox_fn_t *compile(const char *src, struct state *state);

#endif // __CLOX_COMPILER_H__