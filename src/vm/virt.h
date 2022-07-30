/**
 * @file virt.h
 * @author Dylan Mayor
 * @brief header file for virtual machine implementation
 */
#ifndef __CLOX_VM_VIRT_H__
#define __CLOX_VM_VIRT_H__

#include "chunk/chunk.h"
#include "state/state.h"
#include "util/list/list.h"

//! @brief number of stack slots reserved by the vm
#define STACK_RESERVED_COUNT 1
//! @brief stack main function/script index
#define STACK_MAIN_IDX 0

//! @brief call frame for lox functions
struct vm_call_frame {
	lox_fn_t *fn;
	uint8_t *ip;
	size_t stack_snapshot;
};

//! @brief vm struct
typedef struct __vm {
	list_t frames;
	struct state state;
	list_t globals;
	list_t stack;
#ifdef DEBUG_BENCH
	hashmap_t timings_map;
#endif
} vm_t;

//! @brief vm return results
enum vm_res {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR,
};

/**
 * @brief initializes a new virtual machine
 *
 * @return vm_t the new virtual machine
 */
vm_t vm_init();

/**
 * @brief begins interpreting the passed lox source code
 *
 * @param vm the vm to interpret with
 * @param src the source code to interpret
 * @return enum vm_res the vm result
 */
enum vm_res vm_interpret(vm_t *vm, const char *src);

/**
 * @brief frees the passed vm and its related objects
 *
 * @param vm the vm to free
 */
void vm_free(vm_t *vm);

/**
 * @brief prints the variables currently held by the vm
 *
 * @param vm the vm
 */
void vm_print_vars(vm_t *vm);

/**
 * @brief prints the current vm stack
 *
 * @param vm the vm
 */
void vm_print_stack(vm_t *vm);

#endif // __CLOX_VM_VIRT_H__