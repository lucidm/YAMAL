#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <allocator.h>

#define MEMSIZE (16*1024)

#define MAXALLOCATIONS 10
#define MAXSIZE (MEMSIZE / MAXALLOCATIONS)

char* allocations[MAXALLOCATIONS];
uint16_t rn[MAXALLOCATIONS];

int randr(int min, int max)
{
    return (rand() % max) + min;
}

void init()
{
    for(int i=0; i<MAXALLOCATIONS; i++)
	rn[i] = i;
}

void shuffle()
{
    for(int i = MAXALLOCATIONS-1; i > 0; i--)
    {
	int j = randr(0, i+1);
	int tmp = rn[j];
        rn[j] = rn[i];
	rn[i] = tmp;
    }
}

uint8_t *_a_heapstart;
size_t _a_heapsize;

int main(void)
{
    _a_heapstart = malloc(MEMSIZE);
    _a_heapsize = MEMSIZE;

    init();
    shuffle();
    _amalloc(0);
    _printAllocs(NULL);

    for(int i=0; i<MAXALLOCATIONS; i++)
    {
	allocations[i] = _amalloc(randr(10,MAXSIZE));
        if(allocations[i] == NULL)
            printf("Cannot alloc block %u\n", i);
    }
_printAllocs(NULL);
    for(int i=0; i<MAXALLOCATIONS/2; i++)
    {
        _afree(allocations[rn[i]]);
        allocations[rn[i]] = NULL;
    }
_printAllocs(NULL);

    uint32_t s = _a_heapsize;
    uintptr_t *p;
    for(int i=MAXALLOCATIONS/2; i<MAXALLOCATIONS; i++)
    {
        int k = rn[i], r = randr(10,MAXSIZE);
        printf("------------------------------------------------\n");
        p = allocations[k];
        _printAllocs(p);
        allocations[k] = _arealloc(allocations[k], r);
        _printAllocs(p);
        _printAllocs(allocations[k]);


        printf("Tried realloc block %d-%d size %u", i, k, r);
        printf(" orginial 0x%08LX ", OFFSET(p, -SSIZE));
        printf(" realocated 0x%08LX\n\n", OFFSET(allocations[k], -SSIZE));

        printf("------------------------------------------------\n");
        if(allocations[k] == NULL)
        {
            printf("Cannot realloc block %u with size %u\n", i, r);
            s = r;
        }
    }

    _printAllocs(NULL);



    free(_a_heapstart);
    return 0;
}
