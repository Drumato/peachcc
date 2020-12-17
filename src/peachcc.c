#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int read_file(const char *file_path, char **buf);

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: ./peachcc <file-name>\n");
        exit(1);
    }
    const char *file_path = argv[1];
    char *p;
    int status;
    if ((status = read_file(file_path, &p)) != 0)
    {
        fprintf(stderr, "read from %s failed.\n", file_path);
        exit(1);
    }

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    printf("  mov rax, %ld\n", strtol(p, &p, 10));

    while (*p)
    {
        if (*p == '\n')
        {
            p++;
            continue;
        }
        if (*p == '+')
        {
            p++;
            printf("  add rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        if (*p == '-')
        {
            p++;
            printf("  sub rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        fprintf(stderr, "unexpected char: '%c'\n", *p);
        return 1;
    }

    printf("  ret\n");
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