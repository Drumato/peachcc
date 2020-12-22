#include "peachcc.h"

#include <stdlib.h>

//!新しいトークンを作成する
Token *new_token(TokenKind kind, char *str, size_t length)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->length = length;
    return tok;
}

// 識別子トークンの作成
Token *new_identifier_token(char *str, size_t length)
{
    Token *tok = new_token(TK_IDENTIFIER, str, length);
    tok->length = length;
    return tok;
}

// 整数トークンの作成
Token *new_integer_token(char *str, int value, size_t length)
{
    Token *tok = new_token(TK_INTEGER, str, length);
    tok->value = value;
    return tok;
}

void push_token(TokenList *tokens, Token *tok)
{
    vec_push(tokens, tok);
}
// リスト中のposが指す現在の要素を見る
Token *current_token(TokenList *tokens)
{
    return (Token *)tokens->data[tokens->pos];
}
// リスト中のposが指す現在のトークンの種類を見る
TokenKind current_tk(TokenList *tokens)
{
    Token *cur;
    cur = current_token(tokens);
    return cur->kind;
}

// トークンを読みすすめる
void progress(TokenList *tokens)
{
    tokens->pos++;
}