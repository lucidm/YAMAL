CC=gcc
CFLAGS=-g -pg -Og
LFLAGS=
OBJS=allocator.o


all: $(OBJS) allocatortest

allocatortest: main.c $(OBJS)
	$(CC) $(CFLAGS) -I./ $^ -o $@ $(LFLAGS)

$(OBJS): %.o: %.c %.h
	$(CC) $(CFLAGS) $(LFLAGS) -I./ -c $< -o $@

clean:
	rm -f *.o allocatortest *.out

.PHONY: all clean
