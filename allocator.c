/*
 * allocator.c
 * Implementation of malloc/free/realloc/calloc functions
 * All allocated blocks will be increased in size by
 * header which will make sngly linked list of memory blocks.
 * 
 * Author: Jarek Zok <jarekzok@gmail.com>
 * Licence: MIT https://opensource.org/licenses/MIT
 * 
 * Github: https://github.com/lucidm
 * 
 */

#include "allocator.h"

#ifdef ALLOCATOR_USEREPORT
#include <stdio.h>
#endif

static t_MemNode *firstblock = NULL, *lastblock = NULL;

/**
 * \brief Naive implementation of function copying memory block.
 * Can be reimplementend in user library for better performance.
 */
void __attribute__((weak)) _acopymem(void *dest, void *ptr, size_t amount)
{
  for(size_t i=0; i < amount; i++)
  {
    ((char*)dest)[i] = (*(char*)ptr + i);
  }
}

/**
 * \brief Joins two adjacent blocks if they lie against each other.
 * 
 * Function doesn't check if two blocks merging have same free/used flags,
 * checking this is up to function calling _joinBlocks().
 * 
 * @param t_MemNode* "left" block - will become new larger block
 * @param t_MemNode* "right" block - will be merged to "left" block
 * @return t_MemNode* address of "left" block or NULL if both blocks aren't adjacent.
 */
static t_MemNode *_joinBlocks(t_MemNode *src, t_MemNode *nxt)
{
  if (nxt && src->next == nxt)
  {
    src->next = nxt->next;
    src->size += nxt->size;
    return src;
  }
  return NULL;
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
  
  if (src && offset > 0 && (offset < (src->size)))
  {
    size_t tmp = src->size;
    src->size = offset + SSIZE;
    next = (t_MemNode*)OFFSET(src, src->size);
    next->size = tmp - (offset + SSIZE);
    next->free = BLOCK_FREE;
    next->next = src->next;
    src->next = next;
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
static void _tieAdjacent(t_MemNode *node)
{
  t_MemNode *ntmp = node;

  while(node)
  {
      ntmp = node->next;
      if (ntmp == NULL)
      {
	lastblock = node;
	break;
      }

      if (node->free == BLOCK_FREE)
	if (ntmp->free == BLOCK_FREE)
	{
	  node->size += ntmp->size;
	  node->next = ntmp->next;
	}

      node = node->next;
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

    size += sizeof(t_MemNode);

    while(node)
    {
        if (node->free == BLOCK_FREE && size <= node->size && node->size < foundsize)
        {
	  found = node;
          foundsize = node->size;
        }
        node = node->next;
    }
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
    uint32_t sizediff;
    t_MemNode *node, *nodenext;
    
    if(size == 0)
    {
      if (firstblock == NULL)
      {
        firstblock = (t_MemNode*) _a_heapstart;
        firstblock->next = NULL;
        firstblock->size = _a_heapsize;
        firstblock->free = BLOCK_FREE;
	lastblock = firstblock;
	return (void*)OFFSET(firstblock, SSIZE);
      }
      else
	return NULL;
    }

    if (firstblock == NULL)
    {
        if (size > _a_heapsize)
            return NULL;

        firstblock = (t_MemNode*) _a_heapstart;
        firstblock->next = NULL;
        firstblock->size = (size + sizeof(t_MemNode));
        firstblock->free = ~BLOCK_FREE;

        node = (t_MemNode*) ((((uintptr_t)firstblock) + firstblock->size));
        node->next = NULL;
        node->size = (_a_heapsize - firstblock->size);
        node->free = BLOCK_FREE;
        firstblock->next = node;
	lastblock = node;
        return (void*)OFFSET(firstblock, SSIZE);
    }

    _tieAdjacent(firstblock);
    node = _findSmallestFit(size);
    if (node)
    {
        nodenext = NULL;
        sizediff = node->size - (size + sizeof(t_MemNode));
        if (sizediff)
        {
            nodenext = (t_MemNode*)((uint8_t*) node + (size + sizeof(t_MemNode)));
            nodenext->free = BLOCK_FREE;
            nodenext->size = sizediff;
            nodenext->next = node->next;
        }
        node->free = ~BLOCK_FREE;
        node->size = size + sizeof(t_MemNode);
        node->next = nodenext;
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
void _afree(void *mem)
{
    if (!mem) return;
    
    t_MemNode *node = (t_MemNode*) ((uint8_t*)mem - sizeof(t_MemNode));
    t_MemNode *ntmp = node;

    if (node && node->free != BLOCK_FREE)
    {
        node->free = BLOCK_FREE;
	((char*)OFFSET(node, SSIZE))[0] = ' ';
    }
    _tieAdjacent(firstblock);
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
void *_arealloc(void *ptr, size_t size)
{
  t_MemNode *node = (t_MemNode*) ((uint8_t*)ptr - sizeof(t_MemNode)), *nextnode;
  size_t sizesum;
  
  if (ptr == NULL)
    return _amalloc(size);
  
  if (size == 0 && ptr)
  {
    _afree(ptr);
    return NULL;
  }
  
  if (ptr == NULL && size == 0)
    return _amalloc(0);
    
  
  size += SSIZE;
  
  nextnode = node->next;
  if(nextnode && nextnode->free == BLOCK_FREE)
  {
    sizesum = (node->size + nextnode->size);
    if(sizesum >= size)
    {
      node = _joinBlocks(node, nextnode);
      if (sizesum > size)
	node = _splitBlock(node, size);
      return (void*)OFFSET(node, SSIZE);
    }
  }
  nextnode = _amalloc(size);
  if (nextnode)
  {
    nextnode = (t_MemNode*)OFFSET(nextnode, -SSIZE);
    _acopymem(((char*)nextnode + sizeof(t_MemNode)), ((char*)node + sizeof(t_MemNode)), node->size);
    node = (t_MemNode*)OFFSET(node, SSIZE);
    _afree(node);
    node = nextnode;
  } else
    return NULL;
    
  return (void*)OFFSET(node, SSIZE);
}

/**
 * \brief Prints current memory usage and statistics.
 */
#ifdef ALLOCATOR_USEREPORT
void _printAllocs(void)
{
    t_MemNode *node = firstblock;
    uint16_t cnt = 0, freecnt = 0, alloccnt = 0;
    uint32_t rawfree = 0, rawalloc = 0, freesize = 0, allocsize = 0, totsize = 0, rawtot = 0;

    while(node)
    {
        PRINT("#%d\t%c\tAddress: 0x%08X\tSize: %u/%u\t%s\n",
							cnt,
							((char*) OFFSET(node, SSIZE))[0],
                                                        (uintptr_t)node,
                                                        (node->size - sizeof(t_MemNode)),
                                                        node->size,
                                                        node->free == BLOCK_FREE ? "Free" : "Used");
        if (node->free == BLOCK_FREE)
	{
            freesize += (node->size - sizeof(t_MemNode));
	    rawfree += node->size;
	}
        else
	{
            allocsize += (node->size - sizeof(t_MemNode));
	    rawalloc += node->size;
	}
	rawtot += node->size;
	totsize += (node->size - sizeof(t_MemNode));
        cnt++;
	
        if (node->free == BLOCK_FREE)
            freecnt++;
        else
            alloccnt++;
        node = node->next;
    }
    PRINT("\nSummary:\n\tMemory size: %u in %d blocks, %d (%u/%u) blocks free"
           " %d (%u/%u) used.\n\t%u bytes used for list representation\n\tFirst block 0x%08X, last block 0x%08X\n\n", 
								      _a_heapsize,
                                                                      cnt,
                                                                      freecnt, 
								      freesize, rawfree, 
								      alloccnt,
								      allocsize, rawalloc,
								      (rawfree + rawalloc) - (freesize + allocsize),
								      (uintptr_t)firstblock, (uintptr_t)lastblock);
}
#else
void __attribute__((weak)) _printAllocs(void)
{
  //Stub function
}
#endif
