#include "ast/expr.h"
#include "parser/expression.h"
#include "peachcc.h"
#include "token.h"

// expr
Expr *parse(TokenList *tokens)
{
    // 各ASTノードがトークンへのポインタを持っているので，cur_gのfreeは最後までしてはならない．
    cur_g = calloc(1, sizeof(Token));
    current_token(tokens, cur_g);
    Expr *e = expr(tokens);
    return e;
}