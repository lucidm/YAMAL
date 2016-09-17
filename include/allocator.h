#ifndef __ALLOCATOR_H_
#define __ALLOCATOR_H_

#include <stddef.h>
#include <stdint.h>

#ifdef _cplusplus
extern "C" {
#endif

typedef struct _mem_node t_MemNode;

//Below are two variables declared, which should be defined as globals in code using this lib,
//they cannot be declared as static and should be initialized prior to
//first use of _amalloc(...) function.

/*! \var extern uint8_t *_a_heapstart
 * \brief First dddress of heap memory. Declared and set in code using this lib.
 */
extern uint8_t *_a_heapstart;

/*! \var extern size_t _a_heapsize;
 * \brief Size of a heap. Declared and set in code using this lib.
 */
extern size_t _a_heapsize;

/*! \def ALLOCATOR_ALIGNMENT
 * \brief Alignment of memory addres
 */
#define ALLOCATOR_ALIGNMENT 4


/*! \def ALLOCATOR_USEREPORT
 * \brief Comment this def out if don't want _printAllocs() function
 * in your code. Or add -DALLOCATOR_USEREPORT
 */
#define ALLOCATOR_USEREPORT


/*! \fn void *_amalloc(size_t size)
 * \brief Memory allocation function.
 */
void *_amalloc(size_t size);

/*! \fn void _afree(uintptr_t *ptr)
 * \brief Memory free function.
 */
void _afree(uintptr_t *ptr);

/*! \fn void *_arealloc(uintptr_t *ptr, size_t size)
 * \brief Realloc previously allocated memory
 */
void *_arealloc(uintptr_t *ptr, size_t size);

/*! \fn void _acopymem(void *dest, void *ptr, size_t amount)
 * \brief copy memory block form source to destination
 *        can be overwritten for performance reasons.
 */
void __attribute__((weak)) _acopymem(t_MemNode *dest, t_MemNode *src);

/*! \fn void _printAllocs(void)
 * \brief Prints current memory usage and statistics.
 */
void _printAllocs(uintptr_t *ptr);

#ifdef _cplusplus
}
#endif
#endif
