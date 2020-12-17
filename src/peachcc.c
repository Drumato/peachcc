#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: ./peachcc <file-name>\n");
        exit(1);
    }
    char *f;
    int size, status;
    struct stat s;
    const char *file_name = argv[1];
    int fd = open(file_name, O_RDONLY);

    /* Get the size of the file. */
    if ((status = fstat(fd, &s)) != 0)
    {
        perror("fstat failed");
        exit(1);
    }
    size = s.st_size;

    f = (char *)mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    printf("  mov rax, %d\n", atoi(f));
    printf("  ret\n");
    return 0;
}
