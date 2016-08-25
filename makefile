CC=gcc
CFLAGS=-g -pg -O0 -I./ -I./include
LFLAGS=
LIBOBJS=lib/allocator.o
EXAMPLES=simple heavy

.PHONY: all clean $(LIBOBJS)

all: $(EXAMPLES)

$(EXAMPLES): %: ./examples/%.c $(LIBOBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

$(LIBOBJS): %.o: %.c
	$(CC) $(CFLAGS) $(LFLAGS) -DALLOCATOR_USEREPORT -c $< -o $@

clean:
	rm -f $(LIBOBJS) $(EXAMPLES) *.out


