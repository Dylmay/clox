/**
 * @file ops_table.h
 * @author Dylan Mayor
 * @brief X define header file which holds information about vm operations and their display names.
 * Used to populate op name and function defines whilst keeping opcode information in one place
 *
 * @see ops_name.h
 * @see ops.h
 *
 */
X(OP_NOP, "OP_NOP")
X(OP_CONSTANT, "OP_CONSTANT")
X(OP_CONSTANT_LONG, "OP_CONSTANT_LONG")
X(OP_CLOSURE, "OP_CLOSURE")
X(OP_CLOSURE_LONG, "OP_CLOSURE_LONG")
X(OP_EQUAL, "OP_EQUAL")
X(OP_GREATER, "OP_GREATER")
X(OP_LESS, "OP_LESS")
X(OP_NIL, "OP_NIL")
X(OP_TRUE, "OP_TRUE")
X(OP_FALSE, "OP_FALSE")
X(OP_NOT, "OP_NOT")
X(OP_ADD, "OP_ADD")
X(OP_MOD, "OP_MOD")
X(OP_SUBTRACT, "OP_SUBTRACT")
X(OP_MULTIPLY, "OP_MULTIPLY")
X(OP_DIVIDE, "OP_DIVIDE")
X(OP_NEGATE, "OP_NEGATE")
X(OP_POP, "OP_POP")
X(OP_POP_COUNT, "OP_POP_COUNT")
X(OP_VAR_DEFINE, "OP_VAR_DEFINE")
X(OP_VAR_DEFINE_LONG, "OP_VAR_DEFINE_LONG")
X(OP_VAR_GET, "OP_VAR_GET")
X(OP_VAR_GET_LONG, "OP_VAR_GET_LONG")
X(OP_VAR_SET, "OP_VAR_SET")
X(OP_VAR_SET_LONG, "OP_VAR_SET_LONG")
X(OP_GLOBAL_DEFINE, "OP_GLOBAL_DEFINE")
X(OP_GLOBAL_DEFINE_LONG, "OP_GLOBAL_DEFINE_LONG")
X(OP_GLOBAL_GET, "OP_GLOBAL_GET")
X(OP_GLOBAL_GET_LONG, "OP_GLOBAL_GET_LONG")
X(OP_GLOBAL_SET, "OP_GLOBAL_SET")
X(OP_GLOBAL_SET_LONG, "OP_GLOBAL_SET_LONG")
X(OP_UPVALUE_GET, "OP_UPVALUE_GET")
X(OP_UPVALUE_GET_LONG, "OP_UPVALUE_GET_LONG")
X(OP_UPVALUE_SET, "OP_UPVALUE_SET")
X(OP_UPVALUE_SET_LONG, "OP_UPVALUE_SET_LONG")
X(OP_JUMP, "OP_JUMP")
X(OP_JUMP_IF_FALSE, "OP_JUMP_IF_FALSE")
X(OP_CALL, "OP_CALL")
X(OP_RETURN, "OP_RETURN")