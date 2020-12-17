CFLAGS=-std=c11 -g -static -Wall
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)
	INCLUDE=-Isrc/include
	CC=/usr/bin/clang

peachcc: $(OBJS)
	clang -o peachcc $(OBJS) $(LDFLAGS) $(INCLUDE)

$(OBJS): 

test: peachcc
	./test.sh

clean:
	rm -f peachcc src/*.o *~ tmp*

.PHONY: test clean
