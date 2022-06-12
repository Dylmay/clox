#ifndef __CLOX_OPS_H__
#define __CLOX_OPS_H__

#define CONST_LONG_SZ (sizeof(char) * 3)
#define CONST_LONG_MASK (0x00ffffff)

typedef enum {
	OP_CONSTANT,
	OP_CONSTANT_LONG,
	OP_EQUAL,
	OP_GREATER,
	OP_LESS,
	OP_NIL,
	OP_TRUE,
	OP_FALSE,
	OP_NOT,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_NEGATE,
	OP_PRINT,
	OP_POP,
	OP_GLOBAL_DEFINE,
	OP_RETURN
} op_code_t;

#endif // __CLOX_OPS_H__
