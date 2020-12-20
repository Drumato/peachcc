#include "ast/expr.h"
#include "parser/expression.h"
#include "peachcc.h"
#include "token.h"

// expr
Expr *parse(TokenList *tokens)
{
    cur_g = calloc(1, sizeof(Token));
    current_token(tokens, cur_g);
    Expr *e = expr(tokens);
    free(cur_g);
    return e;
}