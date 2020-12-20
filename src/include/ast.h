#ifndef PEACHCC_AST_H_
#define PEACHCC_AST_H_

#include "token.h"

typedef enum
{
    EX_ADD,         // 加算
    EX_SUB,         // 減算
    EX_MUL,         // 乗算
    EX_DIV,         // 除算
    EX_UNARY_PLUS,  // 単項+
    EX_UNARY_MINUS, // 単項-
    EX_INTEGER,     // 整数リテラル
} ExprKind;

typedef struct Expr Expr;

// 式を表すASTノードの型
struct Expr
{
    Token *tok;    // エラー出力用．
    ExprKind kind; // 式の型
    Expr *lhs;     // 左辺(2つのオペランドを取るノードで使用)
    Expr *rhs;     // 右辺(2つのオペランドを取るノードで使用)

    Expr *unary_op; // 単項演算で使用
    int value;      // kindがND_INTEGERの場合のみ使う
};

Expr *new_unop(ExprKind op, Expr *child_expr, Token *tok);
Expr *new_binop(ExprKind op, Expr *lhs, Expr *rhs, Token *tok);
Expr *new_integer(int value, Token *tok);

#endif