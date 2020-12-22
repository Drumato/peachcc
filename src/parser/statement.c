#include "peachcc.h"

static Stmt *expr_stmt(TokenList *tokens);
static Stmt *return_stmt(TokenList *tokens);
static Stmt *compound_stmt(TokenList *tokens);

Stmt *statement(TokenList *tokens)
{
    switch (cur_g->kind)
    {
    case TK_RETURN:
        return return_stmt(tokens);
    case TK_LBRACKET:
        return compound_stmt(tokens);
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
    Stmt *s = new_stmt(ST_RETURN, loc);
    s->expr = e;
    return s;
}

// '{' statement* '}'
static Stmt *compound_stmt(TokenList *tokens)
{
    char *loc = cur_g->str;

    expect(tokens, TK_LBRACKET);
    Vector *body = new_vec();
    while (!try_eat(tokens, TK_RBRACKET))
    {
        vec_push(body, statement(tokens));
    }
    Stmt *s = new_stmt(ST_COMPOUND, loc);
    s->body = body;
    return s;
}

// expression ';'
static Stmt *expr_stmt(TokenList *tokens)
{
    char *loc = cur_g->str;
    Expr *e = expr(tokens);
    expect(tokens, TK_SEMICOLON);
    Stmt *s = new_stmt(ST_EXPR, loc);
    s->expr = e;
    return s;
}