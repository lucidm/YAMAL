/*
 * allocator.c
 * Implementation of malloc/free/realloc/calloc functions
 * All allocated blocks size is increased by
 * header which will make sngly linked list of memory blocks.
 *
 * Author: Jarek Zok <jarekzok@gmail.com>
 * Licence: MIT https://opensource.org/licenses/MIT
 *
 * Github: https://github.com/lucidm
 *
 */

#include <allocator.h>
#include <allocator_lib.h>
#include <limits.h>
#include <assert.h>

#ifdef ALLOCATOR_USEREPORT
#include <stdio.h>
#endif

static t_MemNode *firstblock = NULL;

void _assert_fail(const char *assertion,
                  const char *file,
                  unsigned int line,
                  const char *function,
                  t_MemNode *node
                 )
{
    fprintf(stderr, "\n------\n"
                    "Fail:\t%s\n"
                    "File:\t%s\n"
                    "Line:\t%u\n"
                    "Function:\t%s\n\n"
                    "Node:\t0x%08X\n"
                    "Size:\t%u\n"
                    "------\n",
                        assertion,
                        file,
                        line,
                        function,
                        node,
                        (node ? node->size : 0));
    while(1);
}

t_MemNode *guard(t_MemNode *node)
{
    if (!node)
        return node;

    intptr_t size = GET_BLOCKSIZE(node);
    uintptr_t addr = (uintptr_t)node;

    _assert(node->size != 0, node);
    _assert(size <= _a_heapsize, node);
    _assert(addr >= (uintptr_t)_a_heapstart, node);
    _assert(addr < ((uintptr_t)_a_heapstart + (uintptr_t)_a_heapsize), node);

    if (node->next)
    {
        node = node->next;
        size = GET_BLOCKSIZE(node);
        addr = (uintptr_t)node;

        _assert(node->size != 0, node);
        _assert(size <= _a_heapsize, node);
        _assert(addr >= (uintptr_t)_a_heapstart, node);
        _assert(addr < ((uintptr_t)_a_heapstart + (uintptr_t)_a_heapsize), node);
    }

    return node->next;
}

uintptr_t _abs(intptr_t v)
{
    intptr_t const mask = v >> sizeof(intptr_t) * CHAR_BIT - 1;
    return (v + mask) ^ mask;
}

/**
 * \brief Naive implementation of function copying memory block.
 * Can be reimplementend in user library for better performance.
 */
void __attribute__((weak)) _acopymem(t_MemNode *dest, t_MemNode *src)
{
  size_t s = ((GET_BLOCKSIZE(dest) - SSIZE) > (GET_BLOCKSIZE(src) - SSIZE) ? (GET_BLOCKSIZE(src) - SSIZE) : (GET_BLOCKSIZE(dest) - SSIZE));
  for(size_t i=0; i < s; i++)
  {
    ((char*)dest+SSIZE)[i] = ((char*)src+SSIZE)[i];
  }
}

/**
 * \brief Joins two adjacent blocks if they lie against each other.
 *
 * Merges two blocks toghether if nxt block is adjacent to src block
 * and both blocks have BLOCK_FREE flag set or src block is used and
 * nxt block has BLOCK_FREE flag set.
 *
 * @param t_MemNode* "left" block - will become new larger block
 * @param t_MemNode* "right" block - will be merged to "left" block
 * @return t_MemNode* address of "left" block or NULL if both blocks aren't adjacent.
 */
static t_MemNode *_joinBlocks(t_MemNode *src, t_MemNode *nxt)
{
  size_t newsize;
  if ((nxt && src->next == nxt) &&
      ((BLOCK_ISUSED(src) && BLOCK_ISFREE(nxt)) ||
       (BLOCK_ISFREE(src) && BLOCK_ISFREE(nxt))))
  {
    newsize = GET_BLOCKSIZE(src) + GET_BLOCKSIZE(nxt);
    src->next = nxt->next;
    SET_BLOCKSIZE(src, newsize);
  }
  return src;
}

/**
 * \brief Splits given block in two and return src address.
 *
 * src block will be splitted at given offset, src block will be
 * trimmed to "offset" size, new block will start right after src
 * and the last one will be marked as free and joined as next to the
 * src block. If offset is grater than size of src block, function
 * return src and src block will stay intact.
 *
 * @param t_MemNode* source block
 * @param size_t offset of byte where
 */
static t_MemNode *_splitBlock(t_MemNode *src, size_t offset)
{
  t_MemNode *next;
  size_t size1, size2;

  if(src && offset > 0)
  {
      size1 = offset;
      if (size1 % ALLOCATOR_ALIGNMENT)
          size1 = size1 + ALLOCATOR_ALIGNMENT - (size1 % ALLOCATOR_ALIGNMENT);
      size2 = GET_BLOCKSIZE(src) - size1;

      if (size2 > SSIZE)
      {
          SET_BLOCKUSED(src, size1);
          next = (t_MemNode*)OFFSET(src, size1);
          SET_BLOCKFREE(next, size2);
          next->next = src->next;
          src->next = next;
      }
  }
  return src;
}

/**
 * \brief _tieAdjacent - consolidates adjacent free memory blocks.
 *
 * Prevents excessive memory fragmentation when malloc and free are heavily called
 * especially with huge amount of small memory blocks are massively freed.
 * Called before each allocation and after each deallocation.
 * This function is static.
 *
 * @param t_MemNode address of block which is starting point in search of free adjacent memory blocks
 */
static void _tieAdjacent(t_MemNode *start, t_MemNode *node)
{
  start = (start ? start : firstblock);
  t_MemNode *ntmp = start;
  guard(start);


  while(start && start != node)
  {

      ntmp = start->next;
      guard(ntmp);

      if (BLOCK_ISFREE(start))
	 start = _joinBlocks(start, ntmp);

      start = start->next;
  }
}

/**
 * \brief Tries to find smallest free memory block.
 *
 * Function traverses list of memory blocks and tries to
 * find memory block size in which requested by size block of memory can fit.
 * This function is static.
 *
 * @param size_t requested size
 * @return t_MemNode address of found memory block or NULL
 */
t_MemNode *_findSmallestFit(size_t size)
{
    uint32_t foundsize = _a_heapsize + 1;
    t_MemNode *node = firstblock, *found = NULL;

    if (size > _a_heapsize)
        return found;

    while(node)
    {
        if (BLOCK_ISFREE(node) && GET_BLOCKSIZE(node) < foundsize && size <= GET_BLOCKSIZE(node))
        {
	  found = node;
          foundsize = GET_BLOCKSIZE(node);
        }
        node = node->next;
    }
    guard(node);
    return found;
}

/**
 * \brief Memory allocation function.
 *
 * Returns address of allocated continuous memory block,
 * or NULL if there is no memory available for given size.
 * If size == 0, function will return NULL or pointer to
 * first block of memory if internal block list wasn't
 * previously initialized. In that case returned address
 * shouldn't  be used as address of allocated block, although can
 * be used as _afree() argument.
 * @param size_t size of memory block needed
 * @return void* address of memory block requested
 */
void *_amalloc(size_t size)
{
    int32_t sizediff;
    t_MemNode *node, *nodenext;

    if (size > _a_heapsize)
        return NULL;

    if (firstblock == NULL)
    {
        firstblock = (t_MemNode*) _a_heapstart;
        firstblock->next = NULL;
        SET_BLOCKFREE(firstblock, _a_heapsize);
        guard(firstblock);
    }


    if(size == 0)
        return (void*)OFFSET(firstblock, SSIZE);

    size += SSIZE;
    size = ALIGN(size);

    node = _findSmallestFit(size);
    if (!node)
    {
        _tieAdjacent(firstblock, NULL);
        node = _findSmallestFit(size);
    }
    guard(node);

     if (node)
     {
        MARK_BLOCKUSED(node);
        node = _splitBlock(node, size);
        guard(node);
        return (void*)OFFSET(node, SSIZE);
     }

    return NULL;
}

/**
 * \brief Memory free function.
 *
 * Frees previously allocated memory block. If block was previously allocated
 * function marks it as free and tries to consolidate freed block with adjacent free blocks
 * if there are any.
 * It's upon user to make sure function get right address of memory to be freed. Otherwise it's
 * almost certain list of blocks will end corrupted or programm will make call to random memory
 * address wich will cause hard fault or other unpredictable consequences.
 *
 * @param void* memory bloc to be freed
 */
void _afree(uintptr_t *mem)
{
    if (!mem) return;

    t_MemNode *node = (t_MemNode*) OFFSET(mem, -SSIZE);

    if (node && BLOCK_ISUSED(node))
    {
        guard(node);
        MARK_BLOCKFREE(node);
        _tieAdjacent(firstblock, NULL);
    }

}

/**
 * \brief Realloc previously allocated memory
 *
 * Allocate memory of given size. If block at ptr is adjacent to the
 * free block, tries to merge current block to adjacent, to make larger
 * block as continuous as possible (rest of the block, if any, is marked as free).
 * If there is no such free adjacent block, function reallocates ptr at new address
 * and makes copy of ptr data at new address. If ptr == NULL function acts like _amalloc(...) and
 * return address of new memory block or NULL.
 * If size == 0 and ptr is not NULL, function work like _afree(...) and return NULL.
 * If size > 0 and ptr is not NULL, function will return same address as given in ptr or
 * new address or NULL when there is no free space for new memory block, in that case
 * memory block from ptr is not freed nor relocated.
 * If both size and ptr are 0/NULL function acts like _amalloc(0) which gives ptr to memory
 * block which can be given as argument to _afree(...) function or NULL.
 *
 */
void *_arealloc(uintptr_t *ptr, size_t size)
{
  t_MemNode *node = (t_MemNode*) OFFSET(ptr, -SSIZE), *nextnode;

  guard(node);

  if (size == 0 && ptr)
  {
    _afree(ptr);
    return NULL;
  }

  if (ptr == NULL && size == 0)
    return _amalloc(0);


  if (ptr == NULL)
    return _amalloc(size);

  nextnode = node;
  size += SSIZE;
  size = ALIGN(size);


  if (GET_BLOCKSIZE(node) >= size)
  {
      node = _splitBlock(node, size);
      return (void*) OFFSET(node, SSIZE);
  }

  //otherwise try to find new block to fit
  nextnode = (t_MemNode*)_amalloc(size);
  if (nextnode)
  {
    nextnode = (t_MemNode*)OFFSET(nextnode, -SSIZE);

    guard(nextnode);
    _acopymem(nextnode, node);

    _afree((uintptr_t*)OFFSET(node, SSIZE));
    MARK_BLOCKUSED(nextnode);
  }

  return (nextnode ? (void*)OFFSET(nextnode, SSIZE) : NULL);
}

/**
 * \brief Prints current memory usage and statistics.
 */
#ifdef ALLOCATOR_USEREPORT
void _printAllocs(uintptr_t *ptr)
{
    t_MemNode *node = firstblock, *cmp = NULL;
    uint16_t cnt = 0, freecnt = 0, alloccnt = 0;
    uint32_t rawfree = 0, rawalloc = 0, freesize = 0, allocsize = 0, totsize = 0, rawtot = 0;

    if (ptr)
        cmp = (t_MemNode*) OFFSET(ptr, -SSIZE);

    while(node)
    {
        guard(node);
        if (cmp == node || !cmp)
        PRINT("#%d\t'%c'\t"
              "Address: 0x%08X %d Next: 0x%08X\t"
              "Size: %u/%u\t"
              "%s %s\n",
                        cnt,
                        ((char*) OFFSET(node, SSIZE))[0],
                        (uintptr_t)node,
                        (uintptr_t)node % ALLOCATOR_ALIGNMENT,
                        (uintptr_t)node->next,
                        (GET_BLOCKSIZE(node) - sizeof(t_MemNode)),
                        GET_BLOCKSIZE(node),
                        BLOCK_ISFREE(node) ? "Free" : "Used",
                        (cmp ? (cmp == node ? "*" : "") : ""));
        if (BLOCK_ISFREE(node))
	{
            freesize += (GET_BLOCKSIZE(node) - SSIZE);
	    rawfree += GET_BLOCKSIZE(node);
	}
        else
	{
            allocsize += (GET_BLOCKSIZE(node) - SSIZE);
	    rawalloc += GET_BLOCKSIZE(node);
	}
	rawtot += GET_BLOCKSIZE(node);
	totsize += (GET_BLOCKSIZE(node) - SSIZE);
        cnt++;

        if (BLOCK_ISFREE(node))
            freecnt++;
        else
            alloccnt++;
        node = node->next;
    }
    if(!cmp)
    PRINT("\nSummary:\n\t"
          "Memory size: %u in %d blocks\n"
          "\t%d blocks (%u/%u) free,%d blocks (%u/%u) used.\n"
          "\t%u bytes used for list representation\n"
          "\tFirst block 0x%08LX\n\n",
                                    _a_heapsize,
                                    cnt,
                                    freecnt,
                                    freesize, rawfree,
                                    alloccnt,
                                    allocsize, rawalloc,
                                    (rawfree + rawalloc) - (freesize + allocsize),
                                    (uintptr_t)firstblock);
}
#else
void __attribute__((weak)) _printAllocs(void)
{
  //Stub function
}
#endif
