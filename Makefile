SRC_DIR ?= ./src

CFLAGS=-std=c11 -g -static -Wall -Isrc/include -Isrc/include
SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS=$(SRCS:.c=.o)
	CC=/usr/bin/clang

peachcc: $(OBJS)
	clang -o peachcc $(OBJS) $(LDFLAGS)

$(OBJS):

test: peachcc
	./test.sh

clean:
	rm -f peachcc src/*.o src/parser/*.o src/ast/*.o *~ tmp* asm.s

.PHONY: test clean
