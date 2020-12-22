#include "peachcc.h"

static Stmt *expr_stmt(TokenList *tokens);
static Stmt *return_stmt(TokenList *tokens);
static Stmt *compound_stmt(TokenList *tokens);
static Stmt *if_stmt(TokenList *tokens);
static Stmt *for_stmt(TokenList *tokens);

Stmt *statement(TokenList *tokens)
{
    switch (cur_g->kind)
    {
    case TK_RETURN:
        return return_stmt(tokens);
    case TK_LBRACKET:
        return compound_stmt(tokens);
    case TK_IF:
        return if_stmt(tokens);
    case TK_FOR:
        return for_stmt(tokens);
    default:
        return expr_stmt(tokens);
    }
}

// "return" expression ';'
static Stmt *return_stmt(TokenList *tokens)
{
    char *loc = cur_g->str;
    expect(tokens, TK_RETURN);
    Expr *e = expression(tokens);
    expect(tokens, TK_SEMICOLON);
    Stmt *s = new_stmt(ST_RETURN, loc);
    s->expr = e;
    return s;
}

// if" '(' expression ')' statement ("else" statement)?
static Stmt *if_stmt(TokenList *tokens)
{
    char *loc = cur_g->str;
    expect(tokens, TK_IF);
    expect(tokens, TK_LPAREN);
    Expr *condition = expression(tokens);
    expect(tokens, TK_RPAREN);

    Stmt *then = statement(tokens);
    Stmt *if_s = new_stmt(ST_IF, loc);
    if_s->cond = condition;
    if_s->then = then;

    if (!try_eat(tokens, TK_ELSE))
    {
        return if_s;
    }
    if_s->els = statement(tokens);

    return if_s;
}

// "for" '(' expression? ';' expression? ';' expression?')' statement
static Stmt *for_stmt(TokenList *tokens)
{
    char *loc = cur_g->str;

    expect(tokens, TK_FOR);
    expect(tokens, TK_LPAREN);
    Stmt *s = new_stmt(ST_FOR, loc);
    if (!try_eat(tokens, TK_SEMICOLON))
    {
        s->init = expression(tokens);
        expect(tokens, TK_SEMICOLON);
    }
    if (!try_eat(tokens, TK_SEMICOLON))
    {
        s->cond = expression(tokens);
        expect(tokens, TK_SEMICOLON);
    }
    if (!try_eat(tokens, TK_RPAREN))
    {
        s->inc = expression(tokens);
        expect(tokens, TK_RPAREN);
    }

    s->then = statement(tokens);

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
    Expr *e = expression(tokens);
    expect(tokens, TK_SEMICOLON);
    Stmt *s = new_stmt(ST_EXPR, loc);
    s->expr = e;
    return s;
}