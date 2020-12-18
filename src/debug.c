#include "debug.h"
#include "peachcc.h"

#include <stdio.h>
#include <stdlib.h>

void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - c_program_g;
    fprintf(stderr, "%s\n", c_program_g);
    fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}