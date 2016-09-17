LIBOBJS=lib/allocator.o
EXDIR=./examples
EXLIB=$(EXDIR)/lib
EXOBJS=$(EXDIR)/lib/testlib.o
EXAMPLES=$(EXDIR)/simple $(EXDIR)/heavy

CC=gcc
DEFINES=-DALLOCATOR_USEREPORT
CFLAGS=-g -pg -O0 -I./ -I./include -I$(EXLIB)
LFLAGS=

.PHONY: all clean $(LIBOBJS) $(EXOBJS)

all: $(EXAMPLES)

$(EXAMPLES): %: %.c $(EXOBJS) $(LIBOBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

$(EXOBJS): %.o: %.c
	$(CC) $(CFLAGS) $(LFLAGS) $(DEFINES) -c $< -o $@

$(LIBOBJS): %.o: %.c
	$(CC) $(CFLAGS) $(LFLAGS) $(DEFINES) -c $< -o $@

clean:
	rm -f $(LIBOBJS) $(EXAMPLES) $(EXOBJS) *.out


