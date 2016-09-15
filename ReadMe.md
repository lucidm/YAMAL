# YAMAL

YAMAL is abbraviation of Yet Another Memory Allocation Library. Putting it simple, every code in C needs RAM, which can be statically allocated during compilation (or properly - linking) process, or it can be allocated dynamically from heap while your code runs. This library will help you make latter case easier.
You're probably familiar with functions such as [**malloc**](https://en.wikipedia.org/wiki/C_dynamic_memory_allocation) or **free**, this library implements these functions using singly linked list of nodes, each one describing your consecutive malloc call. It was designed to work on bare metal systems (so MCUs mainly), with some simple OS or scheduler running or without any OS, just bare metal. And is iplememented in pure C. There are some places where  usage of dynamic memory alloaction is not recommended or stricly forbidden, like in atomotive industry for example or in general where dynamic allocation will rise risk of failure which may cause catastrophic effects. All objects, variables or structures are statically allocated then, lowering the risk of unpredictable behaviour of software.


## Why linked list?
Because library allocation function can traverse it while you request new free block of RAM and trying to fit smallest free block possible to your requested malloc size. Also it can maintain fragmentation of heap during this process.

## What about fragmentation?
Excessive calls of malloc/free functions during the execution of code, leaves whole heap fragmented, escpecially, when the system you're using don't have any hardware for virtual memory management, where  you can realloc already allocated block of RAM and map it's physical address to the same virtual address. In the end, trying to allocated another chunk of RAM, you can get refusal to your malloc request, even if there are some free blocks, but none of them can fit your data. This library partially prevents this situation by joining two or more adjacent free blocks together. This isn't however a perfect process, and in extreme case, it will leave the heap in state, where no free blocks can be joined together. There are more clever methods to prevent this, however not very time effective nor memory effective. Jumping from node to node generates time cost, also this library isn't speed demon. Node which holds information about size, allocation status and pointer to next node requires some addtitonal RAM for each block. So practically you are not allocating block of RAM with requested size, but requested size + sizeof(struct node) - and some additional bytes for making next block aligned, which will be mentioned later.

## Is it fast?
No it's not, it never meant to be fast. You allocating all required structures at the begining of the code, and their usage spans for the whole life of your code. Or at least memory isn't allocated/freed too often.

## Where it can find use?
It was meant to work with small systems with tiny amounts of RAM, where heap doesn't exceed few kilobytes or megabytes at most. It has two versions, first defines structure which holds pointer to next node, allocated size and status of allocations, lets call this branch **master**, and second one which utilize (or abuse if you like) the fact, that we can declare signed size, where sign of the size in node can mean status of the block of RAM (free or used). And this the second branch with fancy name **node_struct_optm** it gives few extra bytes per node free for you to use, but it also has limits. Size int this version is limited (because we have to keep extra bit for sign, making half of available capacity of variable holding size of a block), yet we are still talking about KBs or MB of RAM, so we have plenty of space in the variable. So any systems with small amount of RAM can use this library as base for memory allocation.

## Libary API
Library implements three main functions:

 - ``` void* _amalloc(size_t)``` - to allocate block of RAM from heap, returns address to actual allocated RAM or NULL
 - ``` void _afree(void*)``` - to free previously allocated block of RAM
 - ``` void *_arealloc(void*, size_t)``` - to realloc previously allocated block of RAM. By the way, it tries to find best fit block size or reuse already allocated block if size is smaller then given block address.```

Library also requires to decalre and set values of two variables
- ```uint8_t *_a_heapstart``` - start address of a heap
- ```size_t _a_heapsize```  - size of a heap in bytes

Some MCU architectures, like ARM Cortex for example, requires address of the RAM to be divisible by power of two (divisible by 2,4,8 etc.), so when you look into **allocator.h** file, you will find "ALLOCATOR_ALIGNMENT" define, which you can set to needs of architecture you'll use. How constraint of divisibility by one od the power of two is achieved? By allocating the size of the memory block + size of node structure and if size isn't divisible by power of two set in "ALLOCATOR_ALIGNMENT", then modulo of the size and "ALLOCATOR_ALIGNMENT" is added to the whole size, making it divisible by "ALLOCATOR_ALIGNMENT" value. Practically making next block address properly aligned (size of node structre is always properly aligned - this is the jobs of a compiler). At least in theory ;).

Additional feature is added by following function
- ```void _printAllocs(uintptr_t *ptr)```
which just prints information about current heap structure with all nodes and the data on stderr whichever is definded by you (in case of embedded systems it can be tty, UART, semihosting, LCD screen or whatever) but **only if** "ALLOCATOR_USEREPORT" in **allocator.h** is defined. Otherwise calling this function will result in no action. Parameter given to this function is just address of a memory block which will be additionally signed by '**' and only data about this block will be printed. It can be helpful if you want mark a block specifically at function output.

## Whats about assert function.
This function was redefined as there was need to check if parameters of node are reasonable at once, making better and more informative (in context of this library of course) output in debug mode.

## Why so strange function names
Original version of the library was named allocator, hence allocator.c. Prefix "_a" was added to differentiate library functions from malloc/free/realloc functions normally used. You can however redefine above trio which will call _amalloc/_afree/_arealloc from YAMAL and don't even have to change your code.

## Can it be used in C++
Of course, you only have to redefine "new" and "delete" operators to use _amalloc and _afree respectively

## Examples
Some examples are also provided in addition to the library. "Heavy" which tries to flip this library over by allocating, deallocating, reallocating RAM randomly and checking result of such actions in term of it's consistency. And "simple", which tries if library will do what it suppose to do. Both work under Linux console environment.

## What files are essential?
You only need three files:
- **allocator.c** - main YAMAL code
- **allocator.h** - library header with library API for use in your application
- **allocator_lib.h** - header specifically used in library only. Just for separating library internals from library API.

allocator.c is the main code of library which requires allocator.h and allocator_lib.h making complete library you can later link to your code. Pleas check makefile in project directory how it all fits together.

## Do I really need it?
Not really :), there are many other implementations of malloc in various versions of standard C library. For example newlib use [sbrk()](https://en.wikipedia.org/wiki/Sbrk) function which is really simple and fast as this is simply addtion and substraction with additional heap crossing borders checking. YAMAL however, is not any new invention, it was made just for better than sbrk() maintenace of memory allocation. There are plenty of other libraries available making same functions as YAMAL do, or even using same or more advanced principles. Just try them and find what suits you.

