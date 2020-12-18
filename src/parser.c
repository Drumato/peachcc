#include "parser.h"
#include "debug.h"
#include <stdbool.h>
#include <stdio.h>

// 今のトークンを指すときに使う
// current_token() 等に渡して使用すること
Token cur_g;

// 次のトークンが期待している種類のときには，
// トークンを1つ読み進めて真を返す．それ以外の場合には偽を返す．
bool consume(TokenList *tokens, TokenKind k)
{
    if (current_tk(tokens) != k)
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
        current_token(tokens, &cur_g);
        error_at(cur_g.str, "unexpected token");
        exit(1);
    }
}

// 次のトークンが整数の場合，トークンを1つ読み進めてその数値を返す．
// それ以外の場合にはエラーを報告する．
int expect_integer_literal(TokenList *tokens)
{
    current_token(tokens, &cur_g);
    if (cur_g.kind != TK_INTEGER)
    {
        fprintf(stderr, "expected integer token\n");
        exit(1);
    }
    eat_token(tokens);
    return cur_g.value;
}

bool at_eof(TokenList *tokens)
{
    return current_tk(tokens) == TK_EOF;
}