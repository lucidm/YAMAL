#ifndef __ALLOCATOR_H_
#define __ALLOCATOR_H_

#include <stddef.h>
#include <stdint.h>

/*! \def ALLOCATOR_ALIGNMENT
 * \brief Alignment of memory addres
 */
#define ALLOCATOR_ALIGNMENT 4

/*! \def ALLOCATOR_USEREPORT
 * \brief Comment this def out if don't want _printAllocs() function
 * in your code.
 */
#define ALLOCATOR_USEREPORT

#ifdef _cplusplus
extern "C" {
#endif
  
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

/*! \def ALIGN(to, number)
 * \brief Return a "number" aligned to value "a"
 * 
 * Used to align addresses of memory with given alignment if
 * used architecture requires it.
 */ 
#define ALIGN(number) ((number + ALLOCATOR_ALIGNMENT) & (~ALLOCATOR_ALIGNMENT))

#ifdef ALLOCATOR_USEREPORT
/*! \def PRINT
 * \brief Change this to something else which will work like printf
 * if you're using this lib in embedded env, like "trace_printf" for
 * example. Used in _printAllocs().
 */
#define PRINT printf
#endif

/*! \def BLOCK_FREE
 * \brief Marking of block which is free and available for allocation
 */
#define BLOCK_FREE 1

typedef struct _mem_node t_MemNode;

typedef struct _mem_node
{
    t_MemNode *next;
    size_t  size;
    uint16_t free;
} __attribute__((packed)) t_MemNode;

/*! \fn void *_amalloc(size_t size)
 * \brief Memory allocation function. 
 */
void *_amalloc(size_t size);

/*! \fn void _afree(void *mem)
 * \brief Memory free function.
 */
void _afree(void *mem);

#ifdef ALLOCATOR_USEREPORT
/*! \fn void _printAllocs(void)
 * \brief Prints current memory usage and statistics.
 */
void _printAllocs(void);
#endif
#ifdef _cplusplus
}
#endif
#endif
