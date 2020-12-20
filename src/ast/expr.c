#include "ast/expr.h"

#include "stdlib.h"

static Expr *new_expr(ExprKind k, Token *tok);

Expr *new_binop(ExprKind op, Expr *lhs, Expr *rhs, Token *tok)
{
    Expr *e = new_expr(op, tok);
    e->lhs = lhs;
    e->rhs = rhs;
    return e;
}

Expr *new_unop(ExprKind op, Expr *child_expr, Token *tok)
{
    Expr *e = new_expr(op, tok);
    e->unary_op = child_expr;
    return e;
}

Expr *new_integer(int value, Token *tok)
{
    Expr *e = new_expr(EX_INTEGER, tok);
    e->value = value;
    return e;
}
static Expr *new_expr(ExprKind k, Token *tok)
{
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = k;
    e->tok = tok;
    return e;
}
