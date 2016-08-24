CC=gcc
CFLAGS=-g -pg -Og -I./ -I./include
LFLAGS=
LIBOBJS=./lib/allocator.o 
EXSOBJS=./examples/simple.o
EXAMPLES=simple

all: $(EXAMPLES)

$(EXAMPLES): $(LIBOBJS) $(EXSOBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

$(EXOSBJS): %.o: %.c 
	$(CC) $(CFLAGS) $(LFLAGS) -c $< -o $@

$(LIBOBJS): %.o: %.c
	$(CC) $(CFLAGS) $(LFLAGS) -DALLOCATOR_USEREPORT -c $< -o $@

clean:
	rm -f $(LIBOBJS) $(EXSOBJS) $(EXAMPLES) *.out

.PHONY: all clean
