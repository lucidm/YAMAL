#include <stdio.h>
#include <testlib.h>

#define MEMSIZE 16384

uint8_t *_a_heapstart;
size_t _a_heapsize;
size_t maxallocations;

uint32_t ccnt = 0;
uintptr_t *p, *a;

int main(void)
{
    srand(time(NULL));
    _a_heapstart = malloc(MEMSIZE);
    _a_heapsize = MEMSIZE;

    maxallocations = 200;

    init(maxallocations);
    _amalloc(0);
    _printAllocs(NULL);

    uint32_t blocks = maxallocations / 2;

    while(ccnt < 20000)
    {
        blocks = randr(10, blocks);
        printf("Iteration %d mem %u max block size %u\n", ccnt+1, MEMSIZE, (MEMSIZE / blocks));

        printf("Trying to alloc %d blocks with various size...", maxallocations);
        if(!allocateblocks(blocks))
            _printAllocs(NULL);
        printf("done.\n");

        printf("Checking consistency...");
        if (!consistency(blocks))
            _printAllocs(NULL);
        printf("done.\n");

        printf("Matching pattern...");
        if (!patternmatching(blocks))
            _printAllocs(NULL);
        printf("done.\n");

        printf("Trying realloc...");
        if (!reallocblocks(blocks))
            _printAllocs(NULL);
        printf("done.\n");

        printf("Matching pattern...");
        if (!patternmatching(blocks))
            _printAllocs(NULL);
        printf("done.\n");

        printf("Trying to free all allocated blocks...");
        freeblocks(blocks);
        printf("done.\n\n");

        ccnt++;
    }
    _printAllocs(NULL);
    deinit();
    free(_a_heapstart);
    return 0;
}
