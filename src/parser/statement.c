#include "peachcc.h"

static Stmt *expr_stmt(TokenList *tokens);

Stmt *statement(TokenList *tokens)
{
    return expr_stmt(tokens);
}

// expression ';'
static Stmt *expr_stmt(TokenList *tokens)
{
    char *loc = cur_g->str;
    Expr *e = expr(tokens);
    expect(tokens, TK_SEMICOLON);
    return new_exprstmt(e, loc);
}