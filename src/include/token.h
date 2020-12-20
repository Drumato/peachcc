#ifndef PEACHCC_TOKEN_H
#define PEACHCC_TOKEN_H

#include "array_stack.h"

// Tokenの種類
enum TokenKind
{
    TK_PLUS,    // `+`
    TK_MINUS,   // `-`
    TK_STAR,    // `*`
    TK_SLASH,   // `/`
    TK_LPAREN,  // `(`
    TK_RPAREN,  // `)`
    TK_INTEGER, // 整数
    TK_EOF,     // 入力の終わり
};
typedef enum TokenKind TokenKind;

typedef struct Token Token;

// プログラムの最小単位であるトークンの定義
struct Token
{
    TokenKind kind; // 種類
    int value;      // 整数ノードのみ使用する意味値
    /** メモリ上のソースコードを指すポインタ．
     * 各Tokenがこの文字列を所有しているわけではないので注意．
     **/
    char *str;
};

typedef arraystack_t TokenList;
// 整数トークンの作成
Token *new_integer_token(char *str, int value);
// 新しいトークンを作成する
Token *new_token(TokenKind kind, char *str);
// スタックにトークンをプッシュする
void push_token(TokenList *tokens, Token *tok);
// リスト中のposが指す現在の要素を見る
void current_token(TokenList *tokens, Token *cur);
// リスト中のposが指す現在のトークンの種類を見る
TokenKind current_tk(TokenList *tokens);
// トークンを読みすすめる
void progress(TokenList *tokens);
#endif