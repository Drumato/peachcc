
#include "peachcc.h"

// expr parsers;
Expr *expression(TokenList *tokens);
static Expr *assign(TokenList *tokens);
static Expr *equality(TokenList *tokens);
static Expr *relation(TokenList *tokens);
static Expr *addition(TokenList *tokens);
static Expr *multiplication(TokenList *tokens);
static Expr *prefix_unary(TokenList *tokens);
static Expr *primary(TokenList *tokens);
static Expr *paren_expr(TokenList *tokens);

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

// ('+' | '-')? prefix_unary
static Expr *prefix_unary(TokenList *tokens)
{
    Expr *e;

    if (try_eat(tokens, TK_PLUS))
        e = new_unop(EX_UNARY_PLUS, prefix_unary(tokens), cur_g->str);
    else if (try_eat(tokens, TK_MINUS))
        e = new_unop(EX_UNARY_MINUS, prefix_unary(tokens), cur_g->str);
    else
        e = primary(tokens);
    return e;
}

// paren-expr | integer-literal
static Expr *primary(TokenList *tokens)
{
    if (eatable(tokens, TK_LPAREN))
    {
        return paren_expr(tokens);
    }

    Token *ident_loc;
    LocalVariable *lv;
    if ((ident_loc = try_eat_identifier(tokens)) != NULL)
    {
        Expr *id = new_identifier(ident_loc->str, ident_loc->length);

        // いずれ宣言の場所にコードを移動する
        if ((lv = (LocalVariable *)map_get(local_variables_g, ident_loc->str, ident_loc->length)) == NULL)
        {
            total_stack_size_in_fn_g += 8;
            lv = new_local_var(ident_loc->str, ident_loc->length, total_stack_size_in_fn_g);
            map_put(local_variables_g, ident_loc->str, lv);
        }
        return id;
    }

    char *loc = cur_g->str;
    int value = expect_integer_literal(tokens);
    return new_integer(value, loc);
}

// '(' expr ')'
static Expr *paren_expr(TokenList *tokens)
{
    expect(tokens, TK_LPAREN);
    Expr *e = expression(tokens);
    expect(tokens, TK_RPAREN);

    return e;
}
