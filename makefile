CC=gcc
CFLAGS=-g -pg -Og -I./ -I./include
LFLAGS=
LIBOBJS=lib/allocator.o 

.PHONY: all clean $(LIBOBJS)

all: simple

simple: $(LIBOBJS) ./examples/simple.c
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

$(LIBOBJS): %.o: %.c
	$(CC) $(CFLAGS) $(LFLAGS) -DALLOCATOR_USEREPORT -c $< -o $@

clean:
	rm -f $(LIBOBJS) $(EXAMPLES) *.out


