/**
 * @file val_func.h
 * @author Dylan Mayor
 * @brief header file for lox value-related functions
 *
 */
#ifndef __CLOX_VAL_FUNC_H__
#define __CLOX_VAL_FUNC_H__

#include <math.h>

#include "val/val.h"

/**
 * @brief creates a new lox bool value
 *
 * @see lox_bool_t
 *
 * @param value of the boolean. Non-zero is true
 *
 * @return lox_val_t boolean lox value
 *
 */
#define VAL_CREATE_BOOL(value) ((lox_val_t){ VAL_BOOL, { .boolean = value } })
/**
 * @brief creates a new lox nil value
 *
 * @return lox_val_t nil value
 *
 */
#define VAL_CREATE_NIL ((lox_val_t){ VAL_NIL, { .boolean = false } })

static inline double f_to_number(float float_val)
{
	return isnan(float_val) ? NAN : float_val;
}

static inline double d_to_number(double double_val)
{
	return isnan(double_val) ? NAN : double_val;
}

#define VAL_FORMAT_NUMBER(number)                                              \
	(_Generic((number),                                                    \
		float: f_to_number(number),                                    \
		double: d_to_number(number),                                   \
		default: number))

/**
 * @brief creates a new lox number value
 *
 * @see lox_num_t
 *
 * @param value the float value of the number
 *
 * @return lox_val_t number value
 *
 */
#define VAL_CREATE_NUMBER(value)                                               \
	((lox_val_t){ VAL_NUMBER, { .number = VAL_FORMAT_NUMBER(value) } })

/**
 * @brief creates a new error
 *
 * @param object the error object
 *
 * @return lox_val_t error value
 *
 */
#define VAL_CREATE_ERR(object)                                                 \
	((lox_val_t){ VAL_ERR, { .obj = (lox_obj_t *)object } })
/**
 * @brief creates a new lox object value
 *
 * @see lox_obj_t
 *
 * @param object the pointer to the lox object. Undefined behaviour if the object is not a lox object pointer
 *
 * @return lox_val_t lox object
 *
 */
#define VAL_CREATE_OBJ(object)                                                 \
	((lox_val_t){ VAL_OBJ, { .obj = (lox_obj_t *)object } })

/**
 * @brief gets the given value as a lox boolean
 *
 * @see lox_bool_t
 *
 * @param value the lox value to interpret as a lox bool
 *
 * @return lox_bool_t the boolean
 *
 */
#define VAL_AS_BOOL(value) ((value).as.boolean)
/**
 * @brief gets the given value as a lox number. Undefined behaviour for lox values which are not numbers
 *
 * @see lox_num_t
 *
 * @param value the lox value to interpret as a lox number
 *
 * @return lox_num_t the number
 *
 */
#define VAL_AS_NUMBER(value) ((value).as.number)
/**
 * @brief gets the given value as a lox object. Undefined behaviour for lox values which are not objects
 *
 * @see lox_obj_t
 *
 * @param value the lox value to interpret as a lox object
 *
 * @return lox_obj_t the object
 *
 */
#define VAL_AS_OBJ(value) ((value).as.obj)

/**
 * @brief checks whether the given value is a lox bool
 *
 * @see lox_bool_t
 *
 * @param value the lox value to chedk
 *
 * @return true the lox value is a lox bool
 * @return true the lox value is not a lox bool
 *
 */
#define VAL_IS_BOOL(value) ((value).type == VAL_BOOL)
/**
 * @brief checks whether the given value is a lox nil
 *
 * @param value the lox value to check
 *
 * @return true the lox value is a lox nil
 * @return false the lox value is not a lox ni
 *
 */
#define VAL_IS_NIL(value) ((value).type == VAL_NIL)
/**
 * @brief checks whether the given value is a lox number
 *
 * @param value the lox value to check
 *
 * @return true the lox value is a lox number
 * @return false the lox value is not a lox number
 *
 */
#define VAL_IS_NUMBER(value) ((value).type == VAL_NUMBER)
/**
 * @brief checks whether the given value is a lox object
 *
 * @param value the lox value to check
 *
 * @return true the lox value is a lox object
 * @return false the lox value is not a lox object
 *
 */
#define VAL_IS_OBJ(value) ((value).type == VAL_OBJ)

/**
 * @brief checks whether the given value is a lox error
 *
 * @param value the lox value to check
 *
 * @return true the lox value is a lox error
 * @return false the lox value is not a lox error
 *
 */
#define VAL_IS_ERR(value) ((value).type == VAL_ERR)

/**
 * @brief whether the passed value is falsey
 *
 * @param val the value to test
 * @return true the value is truthy
 * @return false the value is falsey
 */
bool val_is_falsey(lox_val_t val);

/**
 * @brief prints the passed value to stdout
 *
 * @param val the value to print
 */
void val_print(lox_val_t val);

/**
 * @brief whether the passed values are equal
 *
 * @param a value a
 * @param b value b
 * @return true the two values are equal
 * @return false the two values are false
 */
bool val_equals(lox_val_t a, lox_val_t b);

/**
 * @brief converts the passed value to a string representation
 * @param val the value to convert
 * @return the string representation of the value
 */
lox_val_t val_to_string(lox_val_t val);

#endif // __CLOX_VAL_FUNC_H__