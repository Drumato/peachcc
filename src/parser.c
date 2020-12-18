#include "parser.h"
#include <stdbool.h>
#include <stdio.h>

// 次のトークンが期待している種類のときには，
// トークンを1つ読み進めて真を返す．それ以外の場合には偽を返す．
bool consume(TokenList *tokens, TokenKind k)
{
    TokenKind cur_kind = current_tk(tokens);
    if (cur_kind != k)
        return false;

    eat_token(tokens);
    return true;
}

// 次のトークンが期待している記号のときには，トークンを1つ読み進める．
// それ以外の場合にはエラーを報告する．
void expect(TokenList *tokens, TokenKind k)
{
    if (!consume(tokens, k))
    {
        fprintf(stderr, "unexpected token\n");
        exit(1);
    }
}

// 次のトークンが整数の場合，トークンを1つ読み進めてその数値を返す．
// それ以外の場合にはエラーを報告する．
int expect_integer_literal(TokenList *tokens)
{
    Token maybe_intlit;
    current_token(tokens, &maybe_intlit);
    if (maybe_intlit.kind != TK_INTEGER)
    {
        fprintf(stderr, "expected integer token\n");
        exit(1);
    }
    eat_token(tokens);
    return maybe_intlit.value;
}
bool at_eof(TokenList *tokens)
{
    return current_tk(tokens) == TK_EOF;
}