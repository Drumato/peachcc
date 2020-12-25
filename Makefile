SRC_DIR ?= ./src

CFLAGS=-std=c11 -g -static -Wall -Isrc/include
SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS=$(SRCS:.c=.o)
	CC=/usr/bin/clang
	TEST_SRCS=$(wildcard test/*.c)
	TEST_OBJS=$(TEST_SRCS:.c=.exe)
	tmp_c_file := $(shell mktemp)

peachcc: $(OBJS)
	clang -o peachcc $(OBJS) $(LDFLAGS)

$(OBJS):

test/%.exe: peachcc test/%.c
	@$(CC) -E -P -C test/$*.c > $(tmp_c_file)
	@./peachcc -o test/$*.s -i $(tmp_c_file)
	@$(CC) -o $@ test/$*.s
	@./test/$*.exe

test: $(TEST_OBJS)
	@make clean

self-host:

clean:
	@rm -f peachcc $(OBJS) $(TEST_OBJS) test/*.s *~ tmp* *.s

.PHONY: test clean
