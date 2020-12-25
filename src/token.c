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
    Token *tok = new_token(TK_INTEGER_LITERAL, str, length);
    tok->value = value;
    return tok;
}
// 文字列トークンの作成
// 終点クオートの次
Token *new_string_token(char *str, size_t length)
{
    Token *tok = new_token(TK_STRING_LITERAL, str, length);

    // ダブルクオート2つを削除して length - 2分割り当てる
    tok->copied_contents = (char *)calloc(length - 2 + 1, sizeof(char));

    // 中身がなんであれ，文字列リテラルの中身の長さは[始点+1, 終点-1]
    strncpy(tok->copied_contents, str + 1, length - 2);
    tok->copied_contents[length - 2] = 0;
    return tok;
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