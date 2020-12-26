
#include "peachcc.h"

// expr parsers;
Expr *expression(TokenList *tokens);
static Expr *assign(TokenList *tokens);
static Expr *equality(TokenList *tokens);
static Expr *relation(TokenList *tokens);
static Expr *addition(TokenList *tokens);
static Expr *multiplication(TokenList *tokens);
static Expr *prefix_unary(TokenList *tokens);
static Expr *postfix_unary(TokenList *tokens);
static Expr *primary(TokenList *tokens);
static Expr *paren_expr(TokenList *tokens);
static Expr *ident_expr(TokenList *tokens, Token *ident_loc);
static Expr *call_expr(TokenList *tokens, Expr *id);

// assign
Expr *expression(TokenList *tokens)
{
    return assign(tokens);
}

// equality ("=" assign)?
static Expr *assign(TokenList *tokens)
{
    Expr *e = equality(tokens);
    if (try_eat(tokens, TK_ASSIGN))
    {
        char *assign_loc = cur_g->str;
        e = new_binop(EX_ASSIGN, e, assign(tokens), assign_loc);
    }

    return e;
}

// relation ('==' relation | '!=' relation)*
static Expr *equality(TokenList *tokens)
{
    Expr *e = relation(tokens);

    for (;;)
    {
        // `==`
        if (try_eat(tokens, TK_EQ))
        {
            e = new_binop(EX_EQ, e, relation(tokens), cur_g->str);
        }
        // `!=`
        else if (try_eat(tokens, TK_NTEQ))
        {
            e = new_binop(EX_NTEQ, e, relation(tokens), cur_g->str);
        }
        else
        {
            break;
        }
    }

    return e;
}

// addition ('<' addition | '<=' addition | '>' addition | '>=' addition)*
static Expr *relation(TokenList *tokens)
{
    Expr *e = addition(tokens);

    for (;;)
    {
        // `<`
        if (try_eat(tokens, TK_LE))
        {
            e = new_binop(EX_LE, e, addition(tokens), cur_g->str);
        }
        // `<=`
        else if (try_eat(tokens, TK_LEEQ))
        {
            e = new_binop(EX_LEEQ, e, addition(tokens), cur_g->str);
        }
        // `>`
        else if (try_eat(tokens, TK_GE))
        {
            e = new_binop(EX_GE, e, addition(tokens), cur_g->str);
        }
        // `>=`
        else if (try_eat(tokens, TK_GEEQ))
        {
            e = new_binop(EX_GEEQ, e, addition(tokens), cur_g->str);
        }
        else
        {
            break;
        }
    }
    return e;
}

// multiplication ('+' multiplication | '-' multiplication)*
static Expr *addition(TokenList *tokens)
{
    Expr *e = multiplication(tokens);

    for (;;)
    {
        // `+`
        if (try_eat(tokens, TK_PLUS))
        {
            e = new_binop(EX_ADD, e, multiplication(tokens), cur_g->str);
        }
        // `-`
        else if (try_eat(tokens, TK_MINUS))
        {
            e = new_binop(EX_SUB, e, multiplication(tokens), cur_g->str);
        }
        else
        {
            break;
        }
    }

    return e;
}

// prefix_unary ('*' prefix_unary | '/' prefix_unary)*
static Expr *multiplication(TokenList *tokens)
{
    Expr *e = prefix_unary(tokens);

    for (;;)
    {
        // `*`
        if (try_eat(tokens, TK_STAR))
        {
            e = new_binop(EX_MUL, e, prefix_unary(tokens), cur_g->str);
        }
        // `/`
        else if (try_eat(tokens, TK_SLASH))
        {
            e = new_binop(EX_DIV, e, prefix_unary(tokens), cur_g->str);
        }
        else
        {
            break;
        }
    }

    return e;
}

// ('+' | '-' | '*' | '&' | "sizeof" | "++" | "--") prefix_unary) | postfix_unary
static Expr *prefix_unary(TokenList *tokens)
{
    Expr *e;
    char *loc = cur_g->str;

    if (try_eat(tokens, TK_PLUS))
        e = new_unop(EX_UNARY_PLUS, prefix_unary(tokens), loc);
    else if (try_eat(tokens, TK_MINUS))
        e = new_unop(EX_UNARY_MINUS, prefix_unary(tokens), loc);
    else if (try_eat(tokens, TK_STAR))
        e = new_unop(EX_UNARY_DEREF, prefix_unary(tokens), loc);
    else if (try_eat(tokens, TK_AMPERSAND))
        e = new_unop(EX_UNARY_ADDR, prefix_unary(tokens), loc);
    else if (try_eat(tokens, TK_SIZEOF))
        e = new_unop(EX_UNARY_SIZEOF, prefix_unary(tokens), loc);
    else if (try_eat(tokens, TK_INCREMENT))
    {
        // ++x は単に x = x + 1として良い
        e = prefix_unary(tokens);
        e = new_binop(EX_ASSIGN, e, new_binop(EX_ADD, e, new_integer_literal(1, loc), loc), loc);
    }
    else if (try_eat(tokens, TK_DECREMENT))
    {
        // --x は単に x = x - 1として良い
        e = prefix_unary(tokens);
        e = new_binop(EX_ASSIGN, e, new_binop(EX_SUB, e, new_integer_literal(1, loc), loc), loc);
    }
    else
        e = postfix_unary(tokens);
    return e;
}

// primary (('[' expr ']')* | '++' )?
static Expr *postfix_unary(TokenList *tokens)
{
    Token *loc = current_token(tokens);
    Expr *e = primary(tokens);

    if (try_eat(tokens, TK_INCREMENT))
    {
        // i++ は (i = i + 1) - 1という式として見れる
        e = new_binop(EX_SUB, new_binop(EX_ASSIGN, e, new_binop(EX_ADD, e, new_integer_literal(1, loc->str), loc->str), loc->str), new_integer_literal(1, loc->str), loc->str);
        return e;
    }
    if (try_eat(tokens, TK_DECREMENT))
    {
        // i++ は (i = i - 1) + 1という式として見れる
        e = new_binop(EX_ADD, new_binop(EX_ASSIGN, e, new_binop(EX_SUB, e, new_integer_literal(1, loc->str), loc->str), loc->str), new_integer_literal(1, loc->str), loc->str);
        return e;
    }

    while (try_eat(tokens, TK_LBRACKET))
    {
        Expr *idx = expression(tokens);
        expect(tokens, TK_RBRACKET);
        // x[y] は単に *(x + y) として変換してしまう．
        e = new_unop(EX_UNARY_DEREF, new_binop(EX_ADD, e, idx, loc->str), loc->str);
    }

    return e;
}

// paren-expr | integer-literal | string-literal
static Expr *primary(TokenList *tokens)
{
    if (eatable(tokens, TK_LPAREN))
    {
        return paren_expr(tokens);
    }

    Token *ident_loc;
    if ((ident_loc = try_eat_identifier(tokens)) != NULL)
    {
        return ident_expr(tokens, ident_loc);
    }
    char *loc = cur_g->str;
    if (eatable(tokens, TK_INTEGER_LITERAL))
    {
        int value = expect_integer_literal(tokens);
        return new_integer_literal(value, loc);
    }

    char *contents = expect_string_literal(tokens);
    return new_string_literal(contents, loc);
}

// '(' expr ')'
static Expr *paren_expr(TokenList *tokens)
{
    expect(tokens, TK_LPAREN);
    Expr *e = expression(tokens);
    expect(tokens, TK_RPAREN);

    return e;
}

/// identifier | call_expr
static Expr *ident_expr(TokenList *tokens, Token *ident_loc)
{
    Expr *id = new_identifier(ident_loc->str, ident_loc->length);

    if (!eatable(tokens, TK_LPAREN))
    {
        // 普通の識別子
        return id;
    }

    // 呼び出し式のパース
    return call_expr(tokens, id);
}

// '(' (expression (',' expression)* )? ')'
static Expr *call_expr(TokenList *tokens, Expr *id)
{
    id->kind = EX_CALL;
    expect(tokens, TK_LPAREN);
    Vector *params = new_vec();

    while (!try_eat(tokens, TK_RPAREN))
    {
        vec_push(params, expression(tokens));
        if (try_eat(tokens, TK_RPAREN))
        {
            break;
        }
        expect(tokens, TK_COMMA);
    }
    id->params = params;

    return id;
}