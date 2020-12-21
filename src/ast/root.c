#include "peachcc.h"
Program *new_program(void)
{
    Program *program = (Program *)calloc(1, sizeof(Program));
    program->stmts = new_vec();
    return program;
}