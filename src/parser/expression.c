#include "ast/expr.h"
#include "debug.h"
#include "peachcc.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

static bool eatable(TokenList *tokens, TokenKind k);
static bool try_eat(TokenList *tokens, TokenKind k);
static void expect(TokenList *tokens, TokenKind k);
static int expect_integer_literal(TokenList *tokens);

// expr parsers;
Expr *expr(TokenList *tokens);
static Expr *equality(TokenList *tokens);
static Expr *relation(TokenList *tokens);
static Expr *addition(TokenList *tokens);
static Expr *multiplication(TokenList *tokens);
static Expr *prefix_unary(TokenList *tokens);
static Expr *primary(TokenList *tokens);
static Expr *paren_expr(TokenList *tokens);

// equality
Expr *expr(TokenList *tokens)
{
    return equality(tokens);
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
    Token *intlit = cur_g;
    int value = expect_integer_literal(tokens);
    return new_integer(value, intlit->str);
}

// '(' expr ')'
static Expr *paren_expr(TokenList *tokens)
{
    expect(tokens, TK_LPAREN);
    Expr *e = expr(tokens);
    expect(tokens, TK_RPAREN);

    return e;
}

// 次のトークンが期待している種類のときには，
// トークンを1つ読み進めて真を返す．
// 読み進められなかった時は偽を返す．
static bool try_eat(TokenList *tokens, TokenKind k)
{
    if (current_tk(tokens) != k)
        return false;

    progress(tokens);
    current_token(tokens, cur_g);
    return true;
}

// 次のトークンが期待している記号のときには，トークンを1つ読み進める．
// それ以外の場合にはエラーを報告する．
static void expect(TokenList *tokens, TokenKind k)
{
    if (!try_eat(tokens, k))
    {
        error_at(cur_g->str, "unexpected token");
        exit(1);
    }
    current_token(tokens, cur_g);
}

// 次のトークンが整数の場合，トークンを1つ読み進めてその数値を返す．
// それ以外の場合にはエラーを報告する．
static int expect_integer_literal(TokenList *tokens)
{
    if (cur_g->kind != TK_INTEGER)
    {
        fprintf(stderr, "expected integer token\n");
        exit(1);
    }
    int value = cur_g->value;
    progress(tokens);
    current_token(tokens, cur_g);
    return value;
}

// 現在見ているトークンが渡されたkと同じ種類かチェック
static bool eatable(TokenList *tokens, TokenKind k)
{
    return current_tk(tokens) == k;
}