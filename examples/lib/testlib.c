#include <testlib.h>

extern uint8_t *_a_heapstart;
extern size_t _a_heapsize;
struct alloc *allocs;

int randr(int min, int max)
{
    return (rand() % max) + min;
}

void init(size_t maxallocations)
{
    allocs = (struct alloc*) malloc(maxallocations * sizeof(struct alloc));
}

void deinit(void)
{
    if (allocs)
        free(allocs);
}

uint8_t allocateblocks(uint32_t maximum)
{
        uint8_t adone = 1;
        uint32_t s = 0;
        t_MemNode *node;

        for(int i=0; i<maximum; i++)
        {
            s = randr(10,(_a_heapsize / maximum));
            allocs[i].size = s;
            allocs[i].pattern = 'A'+ randr(0, 8);
            allocs[i].address = (char*)_amalloc(s);
            if (allocs[i].address)
            {
                allocs[i].block = (t_MemNode*)OFFSET(allocs[i].address, -SSIZE);
                allocs[i].used = 1;
                node = allocs[i].block;
                for (int j=0; j<allocs[i].size; j++)
                    allocs[i].address[j] = allocs[i].pattern;
            }else
            {
                adone = 0;
                allocs[i].used = 0;
                allocs[i].address = NULL;
                printf("Cannot alloc block %u %u\n", i, s);
            }
        }
        return adone;
}

uint8_t reallocblocks(uint32_t maximum)
{
    uint8_t adone = 1;
    uint32_t s = 0;
    char *p;

    for(int i=0; i<maximum; i++)
    {
        s = randr(10,(_a_heapsize / maximum));
        allocs[i].size = s;
        allocs[i].pattern = 'A'+ randr(0, 8);

        if(allocs[i].address)
        {
            p = (char*)_arealloc(allocs[i].address, s);
        }

        if (p)
        {
            allocs[i].address = p;
            allocs[i].block = (t_MemNode*)OFFSET(allocs[i].address, -SSIZE);
            allocs[i].used = 1;
            for (int j=0; j<allocs[i].size; j++)
                allocs[i].address[j] = allocs[i].pattern;
        }else
        {
            adone = 0;
            allocs[i].used = 0;
            allocs[i].address = NULL;
            printf("Cannot realloc block %u %u\n", i, s);
        }

    }
    return adone;
}

void freeblocks(uint32_t maximum)
{
        for(int i=0; i<maximum; i++)
        {
            if(allocs[i].used) _afree(allocs[i].address);
            allocs[i].size = 0;
            allocs[i].used = 0;
            allocs[i].address = NULL;
            allocs[i].pattern = 0;
            allocs[i].block = NULL;
        }
}

uint8_t consistency(uint32_t maximum)
{
    uint8_t adone = 1;
    t_MemNode *node;
    for(int i=0; i < maximum; i++)
    {
        if (allocs[i].address)
        {
            node = allocs[i].block;
            if (OFFSET(allocs[i].address, -SSIZE) != node)
            {
                adone = 0;
                printf("Block %i has inconsistent address 0x%08X != 0x%08X\n",
                       i, node, OFFSET(allocs[i].address, -SSIZE));
                _printAllocs(node);
            }

            if ((allocs[i].size + SSIZE) != GET_BLOCKSIZE(node))
            {
                //Block may have inconsistent size when split block detects
                //that after the split, adjacent block has not enough space to hold
                //the node structure. Checking condition is below.
                if ((GET_BLOCKSIZE(node) - (allocs[i].size + SSIZE)) > SSIZE)
                {
                    adone = 0;
                    printf("Block %i has inconsistent size %d != %d\n",
                            i, GET_BLOCKSIZE(node), allocs[i].size);
                    _printAllocs(node);
                }
            }

            if ((allocs[i].used && node->size < 0) || (!allocs[i].used && node->size > 0))
            {
                adone = 0;
                printf("Block %i has inconsistent status %u != %u\n",
                       i, node->size, allocs[i].used);
                _printAllocs(node);
            }

            if (node->size == 0)
            {
                adone = 0;
                printf("Block %i has size == 0\n", i);
                _printAllocs(node);
            }
        }
    }
    return adone;
}

uint8_t patternmatching(uint32_t maximum)
{
    uint8_t adone = 1;
    for(int i=0; i<maximum; i++)
        if (allocs[i].used)
        {
            uint8_t pmatch = 1;
            int j;
            for(j=0; j<allocs[i].size; j++)
                if (allocs[i].address[j] != allocs[i].pattern)
                {
                    adone = 0;
                    pmatch = 0;
                    break;
                }
            if (!pmatch)
            {
                printf("Pattern in block %u at byte %u mismatch, is '%c', should be '%c'\n", i, j, allocs[i].address[j], allocs[i].pattern);
                _printAllocs(OFFSET(allocs[i].address, -SSIZE));
            }
        }
        return adone;
}

