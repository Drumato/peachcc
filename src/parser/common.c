#include "peachcc.h"

// 次のトークンが期待している種類のときには，
// トークンを1つ読み進めて真を返す．
// 読み進められなかった時は偽を返す．
bool try_eat(TokenList *tokens, TokenKind k)
{
    if (current_tk(tokens) != k)
        return false;

    progress(tokens);
    cur_g = current_token(tokens);
    return true;
}

// 次のトークンが期待している記号のときには，トークンを1つ読み進める．
// それ以外の場合にはエラーを報告する．
void expect(TokenList *tokens, TokenKind k)
{
    if (!try_eat(tokens, k))
    {
        error_at(cur_g->str, "unexpected token");
        exit(1);
    }
    cur_g = current_token(tokens);
}

// 次のトークンが整数の場合，トークンを1つ読み進めてその数値を返す．
// それ以外の場合にはエラーを報告する．
int expect_integer_literal(TokenList *tokens)
{
    if (cur_g->kind != TK_INTEGER)
    {
        error_at(cur_g->str, "expected integer literal");
    }
    int value = cur_g->value;
    progress(tokens);
    cur_g = current_token(tokens);
    return value;
}

// 現在見ているトークンが渡されたkと同じ種類かチェック
bool eatable(TokenList *tokens, TokenKind k)
{
    return current_tk(tokens) == k;
}

bool at_eof(TokenList *tokens)
{
    return current_tk(tokens) == TK_EOF;
}
Token *try_eat_identifier(TokenList *tokens)
{
    Token *ident_loc = current_token(tokens);
    if (!try_eat(tokens, TK_IDENTIFIER))
    {
        return NULL;
    }

    return ident_loc;
}