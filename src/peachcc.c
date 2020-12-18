#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "lexer.h"
#include "parser.h"
#include "peachcc.h"
#include "token.h"

int read_file(const char *file_path, char **buf);

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: ./peachcc <file-name>\n");
        exit(1);
    }

    const char *file_path = argv[1];
    int status;
    if ((status = read_file(file_path, &c_program_g)) != 0)
    {
        fprintf(stderr, "read from %s failed.\n", file_path);
        exit(1);
    }

    TokenList l;
    tokenize(&l, c_program_g);

    FILE *output_fd;
    if ((output_fd = fopen("asm.s", "w")) == NULL)
    {
        perror("create output_file failed.");
        exit(1);
    }

    fprintf(output_fd, ".intel_syntax noprefix\n");
    fprintf(output_fd, ".globl main\n");
    fprintf(output_fd, "main:\n");
    fprintf(output_fd, "  mov rax, %d\n", expect_integer_literal(&l));

    while (!at_eof(&l))
    {
        if (consume(&l, TK_PLUS))
        {
            fprintf(output_fd, "  add rax, %d\n", expect_integer_literal(&l));
            continue;
        }

        expect(&l, TK_MINUS);
        fprintf(output_fd, "  sub rax, %d\n", expect_integer_literal(&l));
    }

    fprintf(output_fd, "  ret\n");
    return 0;
}

int read_file(const char *file_path, char **buf)
{
    int size, status, fd;
    struct stat s;
    if ((fd = open(file_path, O_RDONLY)) < 0)
    {
        perror("open failed");
        return -1;
    }

    /* Get the size of the file. */
    if ((status = fstat(fd, &s)) != 0)
    {
        perror("fstat failed");
        return -1;
    }
    size = s.st_size;

    if ((*buf = (char *)mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0)) == (char *)-1)
    {
        perror("mmap failed");
        return -1;
    }

    return 0;
}