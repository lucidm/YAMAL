#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <allocator.h>
#include <time.h>

#define MEMSIZE (16*1024)

#define MAXALLOCATIONS 25
#define MAXSIZE (MEMSIZE / MAXALLOCATIONS)

uintptr_t* allocations[MAXALLOCATIONS];
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
uint32_t s = 0, ccnt = 0;
uintptr_t *p, *a;

int main(void)
{
    srand(time(NULL));
    _a_heapstart = malloc(MEMSIZE);
    _a_heapsize = MEMSIZE;

    init();
    shuffle();
    _amalloc(0);
    _printAllocs(NULL);

    while(ccnt < 8)
    {
        printf("Trying to alloc %d blocks with various size\n", MAXALLOCATIONS);
        for(int i=0; i<MAXALLOCATIONS; i++)
        {
            s = randr(10,MAXSIZE);
            allocations[i] = _amalloc(s);
            if(allocations[i] == NULL)
                printf("Cannot alloc block %u %u\n", i, s);
        }
        _printAllocs(NULL);
        printf("Press <enter> to realloc blocks\n");
        getchar();

        printf("Trying realloc allocated blocks with various size\n");
        for(int i=0; i<MAXALLOCATIONS; i++)
        {
            s = randr(10,MAXSIZE);
            p = a =_arealloc(allocations[i], s);
            if(p)
            {
                if (p == allocations[i])
                {
                    printf("Same block reused\n");
                } else if (p!=NULL && p!=allocations[i])
                {
                    printf("Old address 0x%08LX, new address 0x%08LX\n", OFFSET(allocations[i], -SSIZE), OFFSET(p, -SSIZE));
                    _printAllocs(a);
                    _printAllocs(allocations[i]);
                    _printAllocs(p);
                    allocations[i] = p;
                }
            }else
                printf("Cannot realloc block\n");

        }
        _printAllocs(NULL);
        printf("Press <enter> to free all blocks\n");
        getchar();

        printf("Trying to free all allocated blocks\n");
        for(int i=0; i<MAXALLOCATIONS; i++)
            if(allocations[i]) _afree(allocations[i]);
        ccnt++;
        _printAllocs(NULL);
        printf("Press <enter> to continue\n");
        getchar();

    }

    free(_a_heapstart);
    return 0;
}
