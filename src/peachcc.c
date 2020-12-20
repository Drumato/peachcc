#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "codegen.h"
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

    TokenList tokens;
    tokenize(&tokens, c_program_g);
    Expr *e = parse(&tokens);

    FILE *output_file;
    if ((output_file = fopen("asm.s", "w")) == NULL)
    {
        perror("create output_file failed.");
        exit(1);
    }

    codegen(output_file, e);
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