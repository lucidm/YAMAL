#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <allocator.h>

#define MEMSIZE (16*1024)

uint8_t *_a_heapstart;
size_t _a_heapsize;

int main(void)
{
    _a_heapstart = malloc(MEMSIZE);
    _a_heapsize = MEMSIZE;

    char *mem[4];
    printf("---------------------------------------------------\n");
    printf("Case 0 - init internal list\n");
    _amalloc(0);
    _printAllocs();
    printf("---------------------------------------------------\n");
    
    printf("Case 1 - alloc three blocks 'A', 'B', 'C' in that order\n");
    mem[0] = _amalloc(100);
    mem[1] = _amalloc(150);
    mem[2] = _amalloc(100);
    mem[0][0] = 'A';
    mem[1][0] = 'B';
    mem[2][0] = 'C';
    _printAllocs();
    printf("---------------------------------------------------\n");
    
    printf("Case 2 - free block 'B'\n");
    _afree(mem[1]);
    _printAllocs();
    printf("---------------------------------------------------\n");
    
    
    printf("Case 3 - realloc block 'A' to fit in first two\n");
    mem[0] = _arealloc(mem[0], 160);
    _printAllocs();
    printf("---------------------------------------------------\n");
    
    printf("Case 4 - realloc block 'A', forcing use last free block\n");
    mem[0] = _arealloc(mem[0], 350);
    _printAllocs();
    printf("---------------------------------------------------\n");
    
    printf("Case 5 - free block 'C' and block 'A', alloc block 'A' again\n");
    _afree(mem[2]);   
    _afree(mem[0]);
    mem[0] = _amalloc(10);
    mem[0][0] = 'A';
    _printAllocs();
    printf("---------------------------------------------------\n");
    
    free(_a_heapstart);
    return 0;
}
