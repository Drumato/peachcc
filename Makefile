SRC_DIR ?= ./src

CFLAGS=-std=c11 -g -static -Wall -Isrc/include
SRCS := $(wildcard src/*.c)
OBJS=$(SRCS:src/%.c=build/gen0/%.o)

CC=/usr/bin/clang
TEST_SRCS=$(wildcard test/*.c)
TEST_OBJS=$(TEST_SRCS:.c=.exe)
	tmp_c_file := $(shell mktemp)
	GEN0=peachcc_gen0

GEN1_ASMS=$(SRCS:src/%.c=build/gen1/%.s)

# build peachcc by clang
peachcc_gen0: $(OBJS)
	$(CC) -o $(GEN0) $(OBJS) $(LDFLAGS)

build/gen0/%.o : src/%.c src/include/peachcc.h
	$(CC) $(CFLAGS) -c $< -o $@

# compile peachcc by gen0
build/gen1/%.s:
	$(CC) $(CFLAGS) -E -P -C src/$*.c -o build/gen1/$*.c
	./$(GEN0) -i build/gen1/$*.c -o $@

# build peachcc by gen0
peachili_gen1 : $(GEN1_ASMS)
	$(CC) $(CFLAGS) -o peachili_gen1 $(GEN1_ASMS)

self-host: peachcc_gen0 peachili_gen1
	# gen0のコンパイラばビルド済み

test/%.exe: peachcc_gen0 test/%.c
	@$(CC) -E -P test/$*.c > $(tmp_c_file)
	@./$(GEN0) -o test/$*.s -i $(tmp_c_file)
	@$(CC) -o $@ test/$*.s
	@./test/$*.exe

test: $(TEST_OBJS)
	@make clean
	@echo "\e[32mALL TEST PASSED!\e[0m"

clean:
	@rm -f peachcc $(OBJS) $(TEST_OBJS) test/*.s *~ tmp* *.s build/gen1/*.c

.PHONY: test clean
