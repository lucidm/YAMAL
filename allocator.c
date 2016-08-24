#include "allocator.h"
#include <stdio.h>

static t_MemNode *firstblock = NULL, *lastblock = NULL;

/**
 * \brief _tieAdjacent - ties or consolidates adjacent free memory blocks.
 * 
 * Prevents excessive memory fragmentation when malloc and free are heavily called
 * especially with huge amount of small memory blocks. Called before each allocation 
 * and after each deallocation.
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
      {		

	
	if (ntmp->free == BLOCK_FREE)
	{
	  node->size += ntmp->size;
	  node->next = ntmp->next;
	}
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
    uint32_t foundsize = _a_heapsize;
    t_MemNode *node = firstblock, *found = NULL;

    size += sizeof(t_MemNode);

    while(node)
    {
        if (node->free == BLOCK_FREE && size < node->size && node->size < foundsize)
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
 * or NULL if there is no memory available. NULL returned doesn't mean there is
 * no free memory available, only tells, there is no free continuous memory to fit requested block size.
 * 
 * @param size_t size of memory block needed
 * @return void* address of memory block requested
 */
void *_amalloc(size_t size)
{
    uint32_t sizediff;
    t_MemNode *node, *nodenext;

    if (firstblock == NULL)
    {
        if (size > _a_heapsize)
            return NULL;

        firstblock = (t_MemNode*) ((uint32_t)_a_heapstart);
        firstblock->next = NULL;
        firstblock->size = (size + sizeof(t_MemNode));
        firstblock->free = ~BLOCK_FREE;

        node = (t_MemNode*) ((((uint32_t)firstblock) + firstblock->size));
        node->next = NULL;
        node->size = (_a_heapsize - (firstblock->size + sizeof(t_MemNode)));
        node->free = BLOCK_FREE;
        firstblock->next = node;
	lastblock = node;
        return (void*)(((uint32_t)firstblock) + sizeof(t_MemNode));
    }

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
	_tieAdjacent(firstblock);
        return ((uint8_t*)node) + sizeof(t_MemNode);
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
    t_MemNode *node = (t_MemNode*) ((uint8_t*)mem - sizeof(t_MemNode));
    t_MemNode *ntmp = node;

    if (node && node->free != BLOCK_FREE)
    {
        node->free = BLOCK_FREE;
	_tieAdjacent(node);
    }
}

/**
 * \brief Prints current memory usage and statistics.
 */
#ifdef ALLOCATOR_USEREPORT
void _printAllocs(void)
{
    t_MemNode *node = firstblock;
    uint16_t cnt = 0, freecnt = 0, alloccnt = 0;
    uint32_t freesize = 0, allocsize = 0, totsize = 0;

    while(node)
    {
        PRINT("#%d Address: 0x%08X Size: %u/%u %s\n", cnt,
                                                        (uint32_t)node,
                                                        (node->size - sizeof(t_MemNode)),
                                                        node->size,
                                                        node->free ? "" : "Free");
        if (node->free == BLOCK_FREE)
            freesize += (node->size - sizeof(t_MemNode));
        else
            allocsize += (node->size - sizeof(t_MemNode));
	totsize += node->size;
        cnt++;
	
        if (node->free == BLOCK_FREE)
            freecnt++;
        else
            alloccnt++;
        node = node->next;
    }
    PRINT("Summary: Memory size: %u in %d blocks, of which %d are"
           " free and %d are allocated.\n\tFree mem %u, allocated %u.\n\tFirst block 0x%08X, last block 0x%08X\n", totsize + sizeof(t_MemNode),
                                                                      cnt,
                                                                      freecnt,
                                                                      alloccnt, freesize, allocsize, firstblock, lastblock);
}
#endif
