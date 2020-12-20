CFLAGS=-std=c11 -g -static -Wall -Isrc/include
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)
	CC=/usr/bin/clang

peachcc: $(OBJS)
	clang -o peachcc $(OBJS) $(LDFLAGS)

$(OBJS):

test: peachcc
	./test.sh

clean:
	rm -f peachcc src/*.o *~ tmp* asm.s

.PHONY: test clean
