#ifndef __TEST_ALLOCATOR_LIB_H_
#define __TEST_ALLOCATOR_LIB_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <allocator.h>
#include <allocator_lib.h>
#include <time.h>

typedef enum patterns
{
    pat1 = 'A',
    pat2 = 'B',
    pat3 = 'C',
    pat4 = 'D',
    pat5 = 'E',
    pat6 = 'F',
    pat7 = 'G',
    pat8 = 'H',
} tpat;

struct alloc
{
    size_t size;
    char pattern;
    uint8_t used;
    char* address;
    t_MemNode *block;
};


int randr(int min, int max);
void init(size_t maxallocations);
void deinit();

uint8_t allocateblocks(uint32_t maximum);
uint8_t reallocblocks(uint32_t maximum);
void freeblocks(uint32_t maximum);
uint8_t consistency(uint32_t maximum);
uint8_t patternmatching(uint32_t maximum);

#endif
