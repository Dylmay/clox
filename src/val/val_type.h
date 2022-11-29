/**
 * @file val_type.h
 * @author Dylan Mayor
 * @brief header file for clox value type enums.
 * @see val/types/value_table.h value type defines
 * @see val/types/object_table.h object type defines
 *
 */
#ifndef __VAL_TYPE_H__
#define __VAL_TYPE_H__

#define X(a) a,
//! @brief lox value types
enum value_type {
#include "types/value_table.h"
};

//! @brief lox object types
enum object_type {
#include "types/object_table.h"
};
#undef X

#endif // __VAL_TYPE_H__