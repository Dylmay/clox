/**
 * @file val_name.h
 * @author Dylan Mayor
 * @brief header file for retrieving a string op name of a given op code
 *
 * @see val/types/value_table.h
 * @see val/types/object_table.h
 *
 */
#ifndef __VAL_NAME_H__
#define __VAL_NAME_H__

#include "val_type.h"

#define X(a) #a,
//! @brief value name table. SHOULD NOT BE ACCESSED DIRECTLY. @see op_name()
static char *val_type_names[] = {
#include "value_table.h"
};
//! @brief value name table. SHOULD NOT BE ACCESSED DIRECTLY. @see op_name()
static char *obj_type_names[] = {
#include "object_table.h"
};
#undef X

/**
 * @brief gets the displayable value name for a value type
 *
 * @param type the value type to convert
 * @return char* the string representation of the value type
 */
static inline char *val_type_name(enum value_type type)
{
	return val_type_names[type];
}

/**
 * @brief gets the displayable object name for the object type
 *
 * @param type the object type to convert
 * @return char* the string representation of the object type
 */
static inline char *obj_type_name(enum object_type type)
{
	return obj_type_names[type];
}

#endif // __VAL_NAME_H__