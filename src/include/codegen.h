#pragma once
#include "ast/expr.h"
#include <stdio.h>

void codegen(FILE *output_file, Expr *expr);
