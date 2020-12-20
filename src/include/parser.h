#ifndef PEACHCC_PARSER_H_
#define PEACHCC_PARSER_H_

#include "ast.h"
#include "token.h"
#include <stdbool.h>

Expr *parse(TokenList *tokens);
#endif