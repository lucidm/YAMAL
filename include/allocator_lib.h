#ifndef __ALLOCATOR_LIB_H_
#define __ALLOCATOR_LIB_H_

#include <stddef.h>
#include <stdint.h>

#ifdef _cplusplus
extern "C" {
#endif

typedef struct _mem_node t_MemNode;

/*! \def BLOCK_FREE
 * \brief Marking of block which is free and available for allocation
 */
#define BLOCK_FREE 0

#define OFFSET(block, offset) ((uintptr_t)block + (uintptr_t)(offset))
#define SSIZE (sizeof(t_MemNode))

#define MARK_BLOCKFREE(NODE) (NODE->size = -(_abs(NODE->size)))
#define MARK_BLOCKUSED(NODE) (NODE->size = (_abs(NODE->size)))
#define SET_BLOCKFREE(NODE, isize) NODE->size = -isize
#define SET_BLOCKUSED(NODE, isize) NODE->size = isize

#define BLOCK_ISFREE(NODE) (NODE->size < 0)
#define BLOCK_ISUSED(NODE) (NODE->size > 0)

#define GET_BLOCKSIZE(NODE) (_abs(NODE->size))
#define SET_BLOCKSIZE(NODE, isize) (NODE->size = (NODE->size > 0 ? isize : -isize ))

#ifndef NDEBUG
#define _assert(expr, NODE)							\
  ((expr)								\
   ? __ASSERT_VOID_CAST (0)						\
   : _assert_fail (#expr, __FILE__, __LINE__, __ASSERT_FUNCTION, NODE))
#else
#define _assert(exp, node) (expr)
#endif

/*! \def ALIGN(number)
 * \brief Return a "number" aligned to value "a"
 *
 * Used to align addresses of memory with given alignment if
 * used architecture requires it.
 */
#define ALIGN(NUMBER) ((NUMBER % ALLOCATOR_ALIGNMENT) ? \
        ((NUMBER + ALLOCATOR_ALIGNMENT) - (NUMBER % ALLOCATOR_ALIGNMENT)) : NUMBER);

#ifdef ALLOCATOR_USEREPORT
/*! \def PRINT
 * \brief Change this to something else which will work like printf
 * if you're using this lib in embedded env, like "trace_printf" for
 * example. Used in _printAllocs().
 */
#define PRINT printf
#endif


typedef struct _mem_node
{
    t_MemNode *next;
    intptr_t  size;
} __attribute__((packed)) t_MemNode;

uintptr_t _abs(intptr_t v);

void _assert_fail(const char *assertion,
                  const char *file,
                  unsigned int line,
                  const char *function,
                  t_MemNode *node
                 );

#ifdef _cplusplus
}
#endif

#endif
