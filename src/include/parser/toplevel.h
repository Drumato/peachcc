#pragma once
#include "ast/expr.h"
#include "token.h"

Expr *parse(TokenList *tokens);