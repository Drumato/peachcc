#include "peachcc.h"

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
Expr *new_identifier(char *str, size_t length)
{
    Expr *e = new_expr(EX_LOCAL_VAR, str);
    e->copied_name = (char *)calloc(length, sizeof(char));
    strncpy(e->copied_name, str, length);
    e->copied_name[length] = 0;
    e->length = length;
    return e;
}
static Expr *new_expr(ExprKind k, char *str)
{
    Expr *e = (Expr *)calloc(1, sizeof(Expr));
    e->kind = k;
    e->str = str;
    return e;
}
