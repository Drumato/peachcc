#include "peachcc.h"

static Stmt *expr_stmt(TokenList *tokens);
static Stmt *return_stmt(TokenList *tokens);
static Stmt *if_stmt(TokenList *tokens);
static Stmt *for_stmt(TokenList *tokens);
static Stmt *while_stmt(TokenList *tokens);
static Vector *block_item_list(TokenList *tokens);

Stmt *statement(TokenList *tokens)
{
    switch (cur_g->kind)
    {
    case TK_RETURN:
        return return_stmt(tokens);
    case TK_LBRACKET:
    {
        Token *loc = current_token(tokens);

        Stmt *s = new_stmt(ST_COMPOUND, loc->str);
        s->body = compound_stmt(tokens);
        return s;
    }
    case TK_IF:
        return if_stmt(tokens);
    case TK_FOR:
        return for_stmt(tokens);
    case TK_WHILE:
        return while_stmt(tokens);
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

// "while" '(' expression ')' statement
static Stmt *while_stmt(TokenList *tokens)
{
    char *loc = cur_g->str;
    expect(tokens, TK_WHILE);
    expect(tokens, TK_LPAREN);
    Expr *condition = expression(tokens);
    expect(tokens, TK_RPAREN);

    Stmt *then = statement(tokens);
    Stmt *s = new_stmt(ST_WHILE, loc);
    s->cond = condition;
    s->then = then;
    return s;
}

// "if" '(' expression ')' statement ("else" statement)?
static Stmt *if_stmt(TokenList *tokens)
{
    char *loc = cur_g->str;
    expect(tokens, TK_IF);
    expect(tokens, TK_LPAREN);
    Expr *condition = expression(tokens);
    expect(tokens, TK_RPAREN);

    Stmt *then = statement(tokens);
    Stmt *s = new_stmt(ST_IF, loc);
    s->cond = condition;
    s->then = then;

    if (!try_eat(tokens, TK_ELSE))
    {
        return s;
    }
    s->els = statement(tokens);

    return s;
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

// '{' block_item_list '}'
Vector *compound_stmt(TokenList *tokens)
{
    expect(tokens, TK_LBRACKET);

    Vector *body = block_item_list(tokens);

    expect(tokens, TK_RBRACKET);

    return body;
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

// (declaration | statement)*
static Vector *block_item_list(TokenList *tokens)
{
    Vector *body = new_vec();

    while (!eatable(tokens, TK_RBRACKET))
    {
        if (eatable(tokens, TK_INT))
        {
            Token *id = declaration(tokens);
            insert_localvar_to_fn_env(id);
            continue;
        }

        vec_push(body, statement(tokens));
    }
    return body;
}