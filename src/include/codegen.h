#ifndef PEACHCC_CODEGEN_H_
#define PEACHCC_CODEGEN_H_
#include "ast.h"
#include <stdio.h>

void codegen(FILE *output_file, Expr *expr);
#endif