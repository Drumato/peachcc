#include "token.h"

//!新しいトークンを作成する
Token *new_token(TokenKind kind, char *str)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    return tok;
}

// 整数トークンの作成
Token *new_integer_token(char *str, int value)
{
    Token *tok = new_token(TK_INTEGER, str);
    tok->value = value;
    return tok;
}

void push_token(TokenList *tokens, Token *tok)
{
    arraystack_push(tokens, tok);
}
// リスト中のposが指す現在の要素を見る
void current_token(TokenList *tokens, Token *cur)
{
    arraystack_get(tokens, tokens->pos, cur);
}
// リスト中のposが指す現在のトークンの種類を見る
TokenKind current_tk(TokenList *tokens)
{
    Token cur;
    current_token(tokens, &cur);
    return cur.kind;
}

// トークンを読みすすめる
void eat_token(TokenList *tokens)
{
    tokens->pos++;
}