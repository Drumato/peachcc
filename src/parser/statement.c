#include "peachcc.h"

static Stmt *expr_stmt(TokenList *tokens);
static Stmt *return_stmt(TokenList *tokens);

Stmt *statement(TokenList *tokens)
{
    switch (cur_g->kind)
    {
    case TK_RETURN:
        return return_stmt(tokens);
    default:
        return expr_stmt(tokens);
    }
}

// "return" expression ';'
static Stmt *return_stmt(TokenList *tokens)
{
    char *loc = cur_g->str;
    expect(tokens, TK_RETURN);
    Expr *e = expr(tokens);
    expect(tokens, TK_SEMICOLON);
    return new_returnstmt(e, loc);
}

// expression ';'
static Stmt *expr_stmt(TokenList *tokens)
{
    char *loc = cur_g->str;
    Expr *e = expr(tokens);
    expect(tokens, TK_SEMICOLON);
    return new_exprstmt(e, loc);
}