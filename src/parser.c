#include "parser.h"
#include "ast.h"
#include "debug.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

// 今のトークンを指すときに使う
// current_token() 等に渡して使用すること
Token *cur_g;

static bool eatable(TokenList *tokens, TokenKind k);
static bool try_eat(TokenList *tokens, TokenKind k);
static void expect(TokenList *tokens, TokenKind k);
static int expect_integer_literal(TokenList *tokens);

// expr parsers;
static Expr *expr(TokenList *tokens);
static Expr *multiplication(TokenList *tokens);
static Expr *prefix_unary(TokenList *tokens);
static Expr *primary(TokenList *tokens);

// expr
Expr *parse(TokenList *tokens)
{
    cur_g = calloc(1, sizeof(Token));
    current_token(tokens, cur_g);
    Expr *e = expr(tokens);
    free(cur_g);
    return e;
}

// multiplication ('+' multiplication | '-' multiplication)*
static Expr *expr(TokenList *tokens)
{
    Expr *e = multiplication(tokens);

    for (;;)
    {
        if (try_eat(tokens, TK_PLUS))
            e = new_binop(EX_ADD, e, multiplication(tokens), cur_g);
        else if (try_eat(tokens, TK_MINUS))
            e = new_binop(EX_SUB, e, multiplication(tokens), cur_g);
        else
            return e;
    }
}

// primary ('*' primary | '/' primary)*
static Expr *multiplication(TokenList *tokens)
{
    Expr *e = prefix_unary(tokens);

    for (;;)
    {
        if (try_eat(tokens, TK_STAR))
        {
            e = new_binop(EX_MUL, e, prefix_unary(tokens), cur_g);
        }
        else if (try_eat(tokens, TK_SLASH))
        {
            e = new_binop(EX_DIV, e, prefix_unary(tokens), cur_g);
        }
        else
        {
            return e;
        }
    }
}

// ('+' | '-')? primary
static Expr *prefix_unary(TokenList *tokens)
{
    Expr *e;

    if (try_eat(tokens, TK_PLUS))
        e = new_unop(EX_UNARY_PLUS, primary(tokens), cur_g);
    else if (try_eat(tokens, TK_MINUS))
        e = new_unop(EX_UNARY_MINUS, primary(tokens), cur_g);
    else
        e = primary(tokens);
    return e;
}

// paren-expr | integer-literal
static Expr *primary(TokenList *tokens)
{
    if (try_eat(tokens, TK_LPAREN))
    {
        Expr *e = expr(tokens);
        expect(tokens, TK_RPAREN);
        return e;
    }

    return new_integer(expect_integer_literal(tokens), cur_g);
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