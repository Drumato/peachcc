#include "ast/expr.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static Expr *new_expr(ExprKind k, char *str);

Expr *new_binop(ExprKind op, Expr *lhs, Expr *rhs, char *str)
{
    Expr *e = new_expr(op, str);
    e->lhs = lhs;
    e->rhs = rhs;
    return e;
}

Expr *new_unop(ExprKind op, Expr *child_expr, char *str)
{
    Expr *e = new_expr(op, str);
    e->unary_op = child_expr;
    return e;
}

Expr *new_integer(int value, char *str)
{
    Expr *e = new_expr(EX_INTEGER, str);
    e->value = value;
    return e;
}
static Expr *new_expr(ExprKind k, char *str)
{
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = k;
    e->str = str;
    return e;
}
