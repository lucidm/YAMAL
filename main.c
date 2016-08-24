#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include "allocator.h"

#define MEMSIZE (16*1024)

uint8_t *_a_heapstart;
size_t _a_heapsize;

int main(void)
{
    _a_heapstart = malloc(MEMSIZE);
    _a_heapsize = MEMSIZE;

    uint32_t *mem[4];

    mem[0] = (uint32_t*)_amalloc(100);
    mem[1] = (uint32_t*)_amalloc(1024);
    mem[2] = (uint32_t*)_amalloc(120);
    mem[3] = (uint32_t*)_amalloc(11 * 1024);
    _printAllocs();

    printf("Free node #2\n");
    _afree(mem[2]);
    _printAllocs();
    
    printf("Free node #3\n");
    _afree(mem[3]);
    _printAllocs();
    
    printf("Free node #1\n");
    _afree(mem[1]);
    _printAllocs();

    printf("Free node #0\n");
    _afree(mem[0]);
    _printAllocs();
    
    _amalloc(150);
    _printAllocs();
    
    free(_a_heapstart);
    return 0;
}
